import type {CaptureDocument,ExportFormat} from './types';
import {saveFile} from './native';
export async function exportDocument(doc:CaptureDocument,format:ExportFormat,image:string){
  let bytes:Uint8Array;
  if(format==='png') bytes=new Uint8Array(await (await fetch(image)).arrayBuffer());
  else if(format==='pdf'){
    const {PDFDocument}=await import('pdf-lib');const pdf=await PDFDocument.create();
    const picture=await pdf.embedPng(image);const scale=Math.min(1,14400/picture.width,14400/picture.height);
    const page=pdf.addPage([picture.width*scale,picture.height*scale]);page.drawImage(picture,{x:0,y:0,width:picture.width*scale,height:picture.height*scale});
    pdf.setTitle(doc.name);pdf.setProducer('拾字 Snip');bytes=await pdf.save();
  }else if(format==='docx'){
    const {Document,Packer,Paragraph,TextRun,Table,TableRow,TableCell,WidthType}=await import('docx');
    const children:(InstanceType<typeof Paragraph>|InstanceType<typeof Table>)[]=doc.text.split('\n').map(line=>new Paragraph({children:[new TextRun({text:line,font:'Microsoft YaHei',size:22})],spacing:{after:100}}));
    if(doc.table.length) children.push(new Table({width:{size:100,type:WidthType.PERCENTAGE},rows:doc.table.map(row=>new TableRow({children:row.map(text=>new TableCell({children:[new Paragraph(text)]}))}))}));
    bytes=new Uint8Array(await Packer.toArrayBuffer(new Document({sections:[{children}]})));
  }else{
    const ExcelJS=await import('exceljs');const book=new ExcelJS.Workbook();const sheet=book.addWorksheet('识别表格');
    doc.table.forEach(row=>sheet.addRow(row));sheet.columns.forEach(col=>{col.width=22;});
    sheet.getRow(1).font={bold:true};sheet.views=[{state:'frozen',ySplit:1}];
    // All cells stay strings: leading zeros survive and OCR text cannot become a formula.
    bytes=new Uint8Array(await book.xlsx.writeBuffer());
  }
  const clean=doc.name.replace(/[<>:"/\\|?*\u0000-\u001f]/g,'_').slice(0,100)||'截图';
  return saveFile(bytes,`${clean}.${format}`,format);
}
