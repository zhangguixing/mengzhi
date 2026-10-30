param([ValidateSet('capture','ocr')][string]$Mode,[string]$Path)
$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
try {
  if ($Mode -eq 'capture') {
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing
    Add-Type @'
using System;
using System.Drawing;
using System.Windows.Forms;
public class SnipSelector : Form {
  Bitmap frame; Point start; Rectangle selection; bool dragging;
  public Rectangle Selected {get;private set;}
  public SnipSelector(Bitmap image, Rectangle screen) {
    frame=image; Bounds=screen; FormBorderStyle=FormBorderStyle.None; StartPosition=FormStartPosition.Manual;
    TopMost=true; DoubleBuffered=true; Cursor=Cursors.Cross; KeyPreview=true;
    KeyDown+=(s,e)=>{if(e.KeyCode==Keys.Escape){DialogResult=DialogResult.Cancel;Close();}};
    MouseDown+=(s,e)=>{if(e.Button==MouseButtons.Left){start=e.Location;dragging=true;Capture=true;}};
    MouseMove+=(s,e)=>{if(dragging){selection=Rectangle.FromLTRB(Math.Min(start.X,e.X),Math.Min(start.Y,e.Y),Math.Max(start.X,e.X),Math.Max(start.Y,e.Y));Invalidate();}};
    MouseUp+=(s,e)=>{if(!dragging)return;dragging=false;Capture=false;if(selection.Width>=8&&selection.Height>=8){Selected=selection;DialogResult=DialogResult.OK;Close();}};
  }
  protected override void OnPaint(PaintEventArgs e){
    e.Graphics.DrawImageUnscaled(frame,0,0);
    using(var shade=new SolidBrush(Color.FromArgb(95,20,25,35)))e.Graphics.FillRectangle(shade,ClientRectangle);
    if(selection.Width>0&&selection.Height>0){e.Graphics.DrawImage(frame,selection,selection,GraphicsUnit.Pixel);using(var pen=new Pen(Color.FromArgb(36,99,184),2))e.Graphics.DrawRectangle(pen,selection);}
  }
}
'@
    $screen=[System.Windows.Forms.SystemInformation]::VirtualScreen
    $bitmap=[System.Drawing.Bitmap]::new($screen.Width,$screen.Height)
    $graphics=[System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.CopyFromScreen($screen.Location,[System.Drawing.Point]::Empty,$screen.Size)
    $graphics.Dispose()
    $selector=[SnipSelector]::new($bitmap,$screen)
    if($selector.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK){
      $cropped=$bitmap.Clone($selector.Selected,$bitmap.PixelFormat)
      $cropped.Save($Path,[System.Drawing.Imaging.ImageFormat]::Png)
      $cropped.Dispose()
    }
    $selector.Dispose();$bitmap.Dispose();exit 0
  }
  Add-Type -AssemblyName System.Runtime.WindowsRuntime
  $null=[Windows.Storage.StorageFile,Windows.Storage,ContentType=WindowsRuntime]
  $null=[Windows.Graphics.Imaging.BitmapDecoder,Windows.Graphics.Imaging,ContentType=WindowsRuntime]
  $null=[Windows.Media.Ocr.OcrEngine,Windows.Foundation,ContentType=WindowsRuntime]
  function Await($op,[Type]$type){
    $method=[System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {$_.Name -eq 'AsTask' -and $_.IsGenericMethod -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'} | Select-Object -First 1
    $task=$method.MakeGenericMethod($type).Invoke($null,@($op));$task.GetAwaiter().GetResult()
  }
  $file=[System.IO.File]::OpenRead($Path)
  $stream=[System.IO.WindowsRuntimeStreamExtensions]::AsRandomAccessStream($file)
  $decoder=Await ([Windows.Graphics.Imaging.BitmapDecoder]::CreateAsync($stream)) ([Windows.Graphics.Imaging.BitmapDecoder])
  $bitmap=Await ($decoder.GetSoftwareBitmapAsync()) ([Windows.Graphics.Imaging.SoftwareBitmap])
  $engine=[Windows.Media.Ocr.OcrEngine]::TryCreateFromUserProfileLanguages()
  if($null -eq $engine){throw '未找到系统 OCR 引擎，请在 Windows 设置中安装中文或英文语言包。'}
  if($bitmap.PixelWidth -gt [Windows.Media.Ocr.OcrEngine]::MaxImageDimension -or $bitmap.PixelHeight -gt [Windows.Media.Ocr.OcrEngine]::MaxImageDimension){throw '图片尺寸超过系统 OCR 限制，请缩小后识别。'}
  $result=Await ($engine.RecognizeAsync($bitmap)) ([Windows.Media.Ocr.OcrResult])
  $blocks=@(foreach($line in $result.Lines){foreach($word in $line.Words){$b=$word.BoundingRect;@{text=$word.Text;x=$b.X/$bitmap.PixelWidth;y=$b.Y/$bitmap.PixelHeight;width=$b.Width/$bitmap.PixelWidth;height=$b.Height/$bitmap.PixelHeight;confidence=$null}}})
  @{blocks=$blocks;text=(($result.Lines | ForEach-Object { $_.Text }) -join [Environment]::NewLine);engine='Windows OCR';language=$engine.RecognizerLanguage.LanguageTag} | ConvertTo-Json -Depth 5 -Compress
  $bitmap.Dispose();$stream.Dispose();$file.Dispose()
} catch { [Console]::Error.Write($_.Exception.Message); exit 1 }
