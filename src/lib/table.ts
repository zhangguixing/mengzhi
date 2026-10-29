import type {OcrBlock} from './types';
export function parseTsv(text:string):string[][] {
  return text.replace(/^[\r\n]+|[\r\n]+$/g,'').split(/\r?\n/).filter(Boolean).map(line=>line.split('\t'));
}
// Conservative geometry grouping; an editable draft, never a table-model result.
export function blocksToTable(blocks:OcrBlock[]):string[][] {
  if(!blocks.length) return [];
  const sorted=[...blocks].sort((a,b)=>a.y-b.y || a.x-b.x);
  const rows:OcrBlock[][]=[];
  for(const block of sorted){
    const row=rows.find(r=>Math.abs(r[0].y+ r[0].height/2-block.y-block.height/2)<Math.max(r[0].height,block.height)*.6);
    if(row) row.push(block); else rows.push([block]);
  }
  const anchors:number[]=[];
  for(const row of rows) for(const b of row){if(!anchors.some(x=>Math.abs(x-b.x)<.035)) anchors.push(b.x);}
  anchors.sort((a,b)=>a-b);
  if(anchors.length>16) return rows.map(row=>row.sort((a,b)=>a.x-b.x).map(b=>b.text));
  return rows.map(row=>{
    const cells=Array<string>(anchors.length).fill('');
    for(const b of row){const index=anchors.reduce((best,x,i)=>Math.abs(x-b.x)<Math.abs(anchors[best]-b.x)?i:best,0);cells[index]+=(cells[index]?' ':'')+b.text;}
    return cells;
  });
}
export const asTsv=(rows:string[][])=>rows.map(row=>row.join('\t')).join('\n');
