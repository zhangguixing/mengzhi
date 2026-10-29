import {invoke, isTauri} from '@tauri-apps/api/core';
import type {OcrBlock} from './types';
export const desktop = isTauri();
export type NativeImage={image:string;width:number;height:number};
export async function captureScreen(){return invoke<NativeImage|null>('capture_screen');}
export async function recognizeImage(image:string){return invoke<{blocks:OcrBlock[];text:string;engine:string}>('recognize_image',{image});}
export async function saveFile(bytes:Uint8Array,name:string,extension:string):Promise<boolean>{
  if(desktop) return invoke<boolean>('save_export',{bytes:Array.from(bytes),name,extension});
  const url=URL.createObjectURL(new Blob([bytes as BlobPart]));const a=document.createElement('a');a.href=url;a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(url),5000);return true;
}
export async function copyImage(image:string){
  if(desktop) return invoke('copy_image',{image});
  const blob=await (await fetch(image)).blob();await navigator.clipboard.write([new ClipboardItem({'image/png':blob})]);
}
export async function pinImage(image:string,width:number,height:number){return invoke('pin_image',{image,width,height});}
