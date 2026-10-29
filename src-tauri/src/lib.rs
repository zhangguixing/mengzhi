use base64::{engine::general_purpose::STANDARD, Engine};
use image::GenericImageView;
use serde::Serialize;
use std::{collections::HashMap, io::Cursor, process::Command, sync::{atomic::{AtomicBool, AtomicU64, Ordering}, Mutex}, time::Duration};
use tauri::{Emitter, Manager, menu::{Menu, MenuItem}, tray::TrayIconBuilder};
use tauri_plugin_global_shortcut::{GlobalShortcutExt, ShortcutState};

#[derive(Clone, Serialize)]
struct ImagePayload { image: String, width: u32, height: u32 }
#[derive(Default)]
struct AppState { pins: Mutex<HashMap<String, ImagePayload>>, capture_busy: AtomicBool, sequence: AtomicU64 }
fn err(e: impl std::fmt::Display) -> String { e.to_string() }
fn decode_image(data: &str) -> Result<image::DynamicImage,String> {
    if data.len()>36_000_000 { return Err("图片超过 25 MB 限制".into()); }
    let encoded=data.strip_prefix("data:image/png;base64,").or_else(||data.strip_prefix("data:image/jpeg;base64,")).or_else(||data.strip_prefix("data:image/webp;base64,")).ok_or("不支持的图片格式")?;
    let bytes=STANDARD.decode(encoded).map_err(err)?;
    let reader=image::ImageReader::new(Cursor::new(&bytes)).with_guessed_format().map_err(err)?;
    let (w,h)=reader.into_dimensions().map_err(err)?;
    if u64::from(w)*u64::from(h)>50_000_000 {return Err("图片超过 5000 万像素限制".into());}
    image::load_from_memory(&bytes).map_err(err)
}
fn payload(image: image::DynamicImage) -> Result<ImagePayload,String> {
    let (width,height)=image.dimensions();let mut out=Cursor::new(Vec::new());image.write_to(&mut out,image::ImageFormat::Png).map_err(err)?;
    Ok(ImagePayload{image:format!("data:image/png;base64,{}",STANDARD.encode(out.into_inner())),width,height})
}
fn helper_path(app: &tauri::AppHandle, name: &str) -> Result<std::path::PathBuf,String> {
    let bundled=app.path().resource_dir().map_err(err)?.join("helpers").join(name);
    if bundled.exists(){return Ok(bundled)}
    let dev=std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("helpers").join(name);
    if dev.exists(){return Ok(dev)}
    Err("本机识别组件缺失，请重新安装完整安装包".into())
}
#[cfg(target_os="windows")]
fn powershell() -> Command {
    use std::os::windows::process::CommandExt;
    let mut command=Command::new("powershell.exe");command.creation_flags(0x08000000);
    command.args(["-NoLogo","-NoProfile","-NonInteractive","-ExecutionPolicy","Bypass","-File"]);command
}
#[tauri::command]
async fn capture_screen(app: tauri::AppHandle) -> Result<Option<ImagePayload>,String> {
    if app.state::<AppState>().capture_busy.swap(true,Ordering::SeqCst){return Err("截图正在进行".into())}
    let window=app.get_webview_window("main").ok_or("工作台未就绪")?;
    let _=window.hide();let worker=app.clone();
    let result=tauri::async_runtime::spawn_blocking(move || -> Result<Option<ImagePayload>,String> {
        std::thread::sleep(Duration::from_millis(220));
        let dir=tempfile::tempdir().map_err(err)?;let path=dir.path().join("capture.png");
        #[cfg(target_os="macos")]
        let output=Command::new("/usr/sbin/screencapture").args(["-i","-x"]).arg(&path).output().map_err(err)?;
        #[cfg(target_os="windows")]
        let output=powershell().arg(helper_path(&worker,"windows.ps1")?).arg("-Mode").arg("capture").arg("-Path").arg(&path).output().map_err(err)?;
        #[cfg(not(any(target_os="macos",target_os="windows")))]
        return Err("当前仅支持 macOS 和 Windows".into());
        #[cfg(any(target_os="macos",target_os="windows"))]
        {
            let _=&worker;
            if !path.exists() {if !output.status.success()&&!output.stderr.is_empty(){return Err(format!("截图未完成，请检查屏幕录制权限：{}",String::from_utf8_lossy(&output.stderr)))}return Ok(None)}
            let image=image::open(path).map_err(err)?;payload(image).map(Some)
        }
    }).await.map_err(err).and_then(|r|r);
    let _=window.show();let _=window.set_focus();app.state::<AppState>().capture_busy.store(false,Ordering::SeqCst);result
}
#[tauri::command]
async fn recognize_image(app: tauri::AppHandle,image:String) -> Result<serde_json::Value,String> {
    tauri::async_runtime::spawn_blocking(move ||{
        let decoded=decode_image(&image)?;let dir=tempfile::tempdir().map_err(err)?;let path=dir.path().join("ocr.png");decoded.save(&path).map_err(err)?;
        #[cfg(target_os="macos")]
        let output=Command::new(helper_path(&app,"bin/shizi-ocr")?).arg(&path).output().map_err(err)?;
        #[cfg(target_os="windows")]
        let output=powershell().arg(helper_path(&app,"windows.ps1")?).arg("-Mode").arg("ocr").arg("-Path").arg(&path).output().map_err(err)?;
        #[cfg(not(any(target_os="macos",target_os="windows")))]
        return Err("当前平台不支持系统 OCR".into());
        #[cfg(any(target_os="macos",target_os="windows"))]
        {if !output.status.success(){return Err(String::from_utf8_lossy(&output.stderr).to_string())}
        serde_json::from_slice(&output.stdout).map_err(err)}
    }).await.map_err(err)?
}
#[tauri::command]
async fn save_export(bytes:Vec<u8>,name:String,extension:String) -> Result<bool,String> {
    if !["png","pdf","docx","xlsx"].contains(&extension.as_str())||bytes.len()>100_000_000{return Err("无效的导出内容".into())}
    let signature=match extension.as_str(){"png"=>bytes.starts_with(b"\x89PNG\r\n\x1a\n"),"pdf"=>bytes.starts_with(b"%PDF"),_=>bytes.starts_with(b"PK")};
    if !signature{return Err("导出文件格式验证失败".into())}
    let name=std::path::Path::new(&name).file_name().and_then(|n|n.to_str()).unwrap_or("export").to_string();
    let file=rfd::AsyncFileDialog::new().set_file_name(&name).add_filter("导出文件",&[&extension]).save_file().await;
    if let Some(file)=file {file.write(&bytes).await.map_err(err)?;Ok(true)}else{Ok(false)}
}
#[tauri::command]
async fn copy_image(image:String) -> Result<(),String> {
    tauri::async_runtime::spawn_blocking(move ||{let image=decode_image(&image)?.to_rgba8();let data=arboard::ImageData{width:image.width() as usize,height:image.height() as usize,bytes:std::borrow::Cow::Owned(image.into_raw())};arboard::Clipboard::new().map_err(err)?.set_image(data).map_err(err)}).await.map_err(err)?
}
#[tauri::command]
fn pin_image(app:tauri::AppHandle,image:String,width:u32,height:u32)->Result<(),String>{
    let decoded=decode_image(&image)?;
    if decoded.width()!=width||decoded.height()!=height{return Err("图片尺寸不匹配".into())}
    let state=app.state::<AppState>();let id=state.sequence.fetch_add(1,Ordering::SeqCst).to_string();
    {let mut pins=state.pins.lock().map_err(err)?;if pins.len()>=12{return Err("最多同时打开 12 张贴图".into())}pins.insert(id.clone(),ImagePayload{image,width,height});}
    let label=format!("pin-{id}");let scale=(650.0/width as f64).min(500.0/height as f64).min(1.0);
    let window=tauri::WebviewWindowBuilder::new(&app,&label,tauri::WebviewUrl::App(format!("index.html?pin={id}").into()))
      .title("拾字贴图").inner_size((width as f64*scale).max(120.0),(height as f64*scale).max(80.0)).min_inner_size(120.0,80.0).decorations(false).always_on_top(true).skip_taskbar(true).build();
    match window{Ok(w)=>{let a=app.clone();w.on_window_event(move |event|{if matches!(event,tauri::WindowEvent::Destroyed){if let Ok(mut pins)=a.state::<AppState>().pins.lock(){pins.remove(&id);}}});Ok(())},Err(e)=>{state.pins.lock().map_err(err)?.remove(&id);Err(err(e))}}
}
#[tauri::command]
fn get_pin(app:tauri::AppHandle,id:String)->Result<ImagePayload,String>{app.state::<AppState>().pins.lock().map_err(err)?.get(&id).cloned().ok_or("贴图不存在".into())}
fn show_main(app:&tauri::AppHandle){if let Some(window)=app.get_webview_window("main"){let _=window.show();let _=window.set_focus();}}
#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run(){
    tauri::Builder::default().manage(AppState::default())
    .plugin(tauri_plugin_single_instance::init(|app,_,_|show_main(app)))
    .plugin(tauri_plugin_global_shortcut::Builder::new().with_handler(|app,_,event|{if event.state()==ShortcutState::Pressed{show_main(app);let _=app.emit_to("main","request-capture",());}}).build())
    .setup(|app|{
        let capture=MenuItem::with_id(app,"capture","新截图",true,None::<&str>)?;
        let open=MenuItem::with_id(app,"open","打开工作台",true,None::<&str>)?;
        let quit=MenuItem::with_id(app,"quit","退出拾字 Snip",true,None::<&str>)?;
        let menu=Menu::with_items(app,&[&capture,&open,&quit])?;
        let mut rgba=vec![0u8;32*32*4];for y in 0..32{for x in 0..32{let edge=((x>=5&&x<8)||(x>=24&&x<27))&&((y>=5&&y<13)||(y>=19&&y<27))||((y>=5&&y<8)||(y>=24&&y<27))&&((x>=5&&x<13)||(x>=19&&x<27));if edge{let i=(y*32+x)*4;rgba[i..i+4].copy_from_slice(&[60,90,135,255]);}}}
        TrayIconBuilder::new().icon(tauri::image::Image::new_owned(rgba,32,32)).tooltip("拾字 Snip").menu(&menu).on_menu_event(|app,event|{match event.id.as_ref(){"quit"=>app.exit(0),"open"=>show_main(app),"capture"=>{show_main(app);let _=app.emit_to("main","request-capture",());},_=>()}}).build(app)?;
        if let Err(e)=app.global_shortcut().register("CommandOrControl+Shift+2"){eprintln!("截图快捷键注册失败，可使用托盘菜单：{e}");}
        if let Some(window)=app.get_webview_window("main"){let w=window.clone();window.on_window_event(move |event|{if let tauri::WindowEvent::CloseRequested{api,..}=event{api.prevent_close();let _=w.hide();}});}
        Ok(())
    }).invoke_handler(tauri::generate_handler![capture_screen,recognize_image,save_export,copy_image,pin_image,get_pin])
    .run(tauri::generate_context!()).expect("无法启动拾字 Snip");
}
