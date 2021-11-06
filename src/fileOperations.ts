export enum FolderType {
  TemplateFolder,
  ProjectFolder
}

export enum OperationType {
  Copy,
  Encrypt
}

export interface FileOperation {
  From: string,
  To: string,
  Operation: OperationType,
  Folder: FolderType
}