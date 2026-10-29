import type {CaptureDocument} from './types';
export function createSample():CaptureDocument {
  const canvas=document.createElement('canvas'); canvas.width=1040; canvas.height=760;
  const c=canvas.getContext('2d')!;
  c.fillStyle='#fffefa'; c.fillRect(0,0,1040,760);
  c.fillStyle='#2b3340'; c.font='600 29px -apple-system, "PingFang SC", sans-serif'; c.fillText('工作室设备采购清单',76,100);
  c.fillStyle='#77808c'; c.font='17px -apple-system, "PingFang SC", sans-serif';c.fillText('2026 年 10 月  ·  项目筹备 / 内部资料',76,141);
  c.fillStyle='#3f4856';c.font='19px -apple-system, "PingFang SC", sans-serif';
  c.fillText('以下为本月设备采购计划，请核对数量与预算。',76,208);
  const rows=[['设备名称','数量','单价（元）','金额（元）'],['27 英寸显示器','2','1,699','3,398'],['机械键盘','3','399','1,197'],['USB-C 扩展坞','2','259','518'],['合计','7','—','5,113']];
  const xs=[76,455,610,790,964];
  c.fillStyle='#f0f2f3';c.fillRect(76,247,888,58);
  rows.forEach((row,r)=>{
    c.fillStyle=r===0?'#4e5968':'#303b49';c.font=`${r===0||r===4?'600':'400'} 18px -apple-system, "PingFang SC", sans-serif`;
    row.forEach((v,i)=>c.fillText(v,xs[i]+16,283+r*63));
    c.strokeStyle='#e0e4e7';c.beginPath();c.moveTo(76,305+r*63);c.lineTo(964,305+r*63);c.stroke();
  });
  c.fillStyle='#747f8c';c.font='16px -apple-system, "PingFang SC", sans-serif';c.fillText('备注：所有金额均为含税预算，实际采购以审批结果为准。',76,644);
  c.fillStyle='#8b939e';c.font='14px -apple-system, "PingFang SC", sans-serif';c.fillText('示例文档 · 可用于体验文字校对与导出',76,704);
  return {id:'sample',name:'工作室设备采购清单',createdAt:Date.now(),image:canvas.toDataURL('image/png'),width:1040,height:760,annotations:[],text:'工作室设备采购清单\n2026 年 10 月 · 项目筹备 / 内部资料\n\n以下为本月设备采购计划，请核对数量与预算。\n\n备注：所有金额均为含税预算，实际采购以审批结果为准。',table:rows,blocks:[],engine:'内置示例',sample:true};
}
