export type Tool = 'move'|'rectangle'|'ellipse'|'arrow'|'pen'|'text'|'mosaic';
export type Point = {x:number;y:number};
export type Annotation = {id:string;tool:Tool;points:Point[];color:string;width:number;text?:string};
export type OcrBlock = {text:string;x:number;y:number;width:number;height:number;confidence:number|null};
export type CaptureDocument = {id:string;name:string;createdAt:number;image:string;width:number;height:number;annotations:Annotation[];text:string;table:string[][];blocks:OcrBlock[];engine:string;sample?:boolean};
export type ExportFormat = 'docx'|'xlsx'|'pdf'|'png';
