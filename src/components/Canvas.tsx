import {forwardRef,useEffect,useImperativeHandle,useRef,useState} from 'react';
import type {Annotation,CaptureDocument,Point,Tool} from '../lib/types';
export type CanvasApi={image:()=>string};
export function paintAnnotation(c:CanvasRenderingContext2D,a:Annotation){
  const p=a.points[0],q=a.points.at(-1)!;if(!p||!q)return;
  c.save();c.strokeStyle=a.color;c.fillStyle=a.color;c.lineWidth=a.width;c.lineCap='round';c.lineJoin='round';
  const x=Math.min(p.x,q.x),y=Math.min(p.y,q.y),w=Math.abs(q.x-p.x),h=Math.abs(q.y-p.y);
  if(a.tool==='rectangle')c.strokeRect(x,y,w,h);
  if(a.tool==='ellipse'){c.beginPath();c.ellipse(x+w/2,y+h/2,w/2,h/2,0,0,Math.PI*2);c.stroke();}
  if(a.tool==='arrow'){
    const angle=Math.atan2(q.y-p.y,q.x-p.x),size=a.width*4+6;
    c.beginPath();c.moveTo(p.x,p.y);c.lineTo(q.x,q.y);c.stroke();
    c.beginPath();c.moveTo(q.x,q.y);c.lineTo(q.x-size*Math.cos(angle-.45),q.y-size*Math.sin(angle-.45));c.lineTo(q.x-size*Math.cos(angle+.45),q.y-size*Math.sin(angle+.45));c.closePath();c.fill();
  }
  if(a.tool==='pen'){c.beginPath();c.moveTo(p.x,p.y);for(const pt of a.points)c.lineTo(pt.x,pt.y);c.stroke();}
  if(a.tool==='text'){c.font=`500 ${Math.max(22,a.width*7)}px -apple-system, "PingFang SC", "Microsoft YaHei", sans-serif`;c.fillText(a.text||'',p.x,p.y);}
  if(a.tool==='mosaic'&&w>0&&h>0){
    const left=Math.max(0,Math.floor(x)),top=Math.max(0,Math.floor(y)),width=Math.min(Math.ceil(w),c.canvas.width-left),height=Math.min(Math.ceil(h),c.canvas.height-top);
    if(width>0&&height>0){const tmp=document.createElement('canvas');tmp.width=Math.max(1,Math.ceil(width/14));tmp.height=Math.max(1,Math.ceil(height/14));tmp.getContext('2d')!.drawImage(c.canvas,left,top,width,height,0,0,tmp.width,tmp.height);c.imageSmoothingEnabled=false;c.drawImage(tmp,0,0,tmp.width,tmp.height,left,top,width,height);}
  }
  c.restore();
}
export const ImageCanvas=forwardRef<CanvasApi,{doc:CaptureDocument;tool:Tool;color:string;lineWidth:number;zoom:number;textLabel:string;onAdd:(a:Annotation)=>void}>(function ImageCanvas({doc,tool,color,lineWidth,zoom,textLabel,onAdd},api){
  const canvas=useRef<HTMLCanvasElement>(null),host=useRef<HTMLDivElement>(null),base=useRef<HTMLImageElement|null>(null),draft=useRef<Annotation|null>(null),drag=useRef<{x:number;y:number;left:number;top:number}|null>(null);
  const [fit,setFit]=useState(.65);
  const redraw=()=>{const c=canvas.current?.getContext('2d');if(!c||!base.current)return;c.clearRect(0,0,doc.width,doc.height);c.drawImage(base.current,0,0);for(const a of doc.annotations)paintAnnotation(c,a);if(draft.current)paintAnnotation(c,draft.current);};
  useImperativeHandle(api,()=>({image:()=>{redraw();return canvas.current!.toDataURL('image/png');}}));
  useEffect(()=>{let alive=true;base.current=null;const img=new Image();img.onload=()=>{if(alive){base.current=img;redraw();}};img.src=doc.image;return()=>{alive=false;};},[doc.image]);
  useEffect(redraw,[doc.annotations]);
  useEffect(()=>{const obs=new ResizeObserver(([entry])=>setFit(Math.min(1,(entry.contentRect.width-88)/doc.width,(entry.contentRect.height-125)/doc.height)));if(host.current)obs.observe(host.current);return()=>obs.disconnect();},[doc.width,doc.height]);
  const point=(e:{clientX:number;clientY:number}):Point=>{const r=canvas.current!.getBoundingClientRect();return{x:Math.max(0,Math.min(doc.width,(e.clientX-r.left)*doc.width/r.width)),y:Math.max(0,Math.min(doc.height,(e.clientY-r.top)*doc.height/r.height))};};
  return <div className={`canvas-scroll ${tool==='move'?'can-pan':''}`} ref={host} onPointerDown={e=>{
    if(tool==='move'){e.currentTarget.setPointerCapture(e.pointerId);drag.current={x:e.clientX,y:e.clientY,left:e.currentTarget.scrollLeft,top:e.currentTarget.scrollTop};}
  }} onPointerMove={e=>{if(drag.current){e.currentTarget.scrollLeft=drag.current.left+drag.current.x-e.clientX;e.currentTarget.scrollTop=drag.current.top+drag.current.y-e.clientY;}}} onPointerUp={()=>{drag.current=null;}} onPointerCancel={()=>{drag.current=null;}}>
    <div className="paper-stage"><canvas ref={canvas} width={doc.width} height={doc.height} aria-label="截图标注画布" style={{width:doc.width*Math.max(.05,fit)*zoom,height:doc.height*Math.max(.05,fit)*zoom,cursor:tool==='move'?'grab':tool==='text'?'text':'crosshair'}}
      onPointerDown={e=>{
        if(tool==='move'||e.button!==0)return;e.preventDefault();e.currentTarget.setPointerCapture(e.pointerId);
        const p=point(e);draft.current={id:crypto.randomUUID(),tool,color,width:lineWidth,points:[p,p],text:textLabel};redraw();
      }}
      onPointerMove={e=>{
        const a=draft.current;if(!a)return;
        if(a.tool==='pen'){const events=e.nativeEvent.getCoalescedEvents?.()||[e.nativeEvent];a.points.push(...events.map(point));}
        else a.points[1]=point(e);redraw();
      }} onPointerUp={()=>{const a=draft.current;draft.current=null;if(a){const p=a.points[0],q=a.points.at(-1)!;if(a.tool==='text'||Math.hypot(q.x-p.x,q.y-p.y)>2||a.points.length>3)onAdd(a);}redraw();}}
      onPointerCancel={()=>{draft.current=null;redraw();}} />
    </div>
  </div>;
});
