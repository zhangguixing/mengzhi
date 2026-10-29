import {describe,it,expect} from 'vitest';
import {parseTsv,blocksToTable,asTsv} from './table';
import type {OcrBlock} from './types';
const block=(text:string,x:number,y:number):OcrBlock=>({text,x,y,width:.15,height:.03,confidence:null});
describe('editable OCR table',()=>{
  it('keeps empty cells and leading zeros when importing spreadsheet data',()=>{expect(parseTsv('编号\t姓名\t备注\r\n001\t林青\t\r\n002\t\t审核')).toEqual([['编号','姓名','备注'],['001','林青',''],['002','','审核']]);});
  it('preserves row and column positions even if input blocks are unordered',()=>{expect(blocksToTable([block('22',.5,.21),block('数量',.5,.1),block('设备',.1,.1),block('屏幕',.1,.2),block('合计',.1,.3)])).toEqual([['设备','数量'],['屏幕','22'],['合计','']]);});
  it('serializes table text without dropping empty cells',()=>{expect(asTsv([['001','','99'],['002','0','']])).toBe('001\t\t99\n002\t0\t');});
  it('has a real empty state',()=>{expect(blocksToTable([])).toEqual([]);expect(parseTsv('')).toEqual([]);});
});
