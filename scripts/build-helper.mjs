import {mkdirSync,writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
mkdirSync('src-tauri/helpers/bin',{recursive:true});
if(process.platform==='darwin'){
  const target=process.arch==='arm64'?'arm64-apple-macosx12.0':'x86_64-apple-macosx12.0';
  const result=spawnSync('swiftc',['-O','-target',target,'-module-cache-path','.tools/swift-cache','src-tauri/helpers/ocr.swift','-o','src-tauri/helpers/bin/shizi-ocr'],{stdio:'inherit'});
  process.exit(result.status??1);
}else writeFileSync('src-tauri/helpers/bin/README.txt','Windows uses the system OCR helper.\n');
