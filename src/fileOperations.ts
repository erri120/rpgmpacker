import Path from "./io/Path";

export enum FolderType {
  TemplateFolder,
  ProjectFolder
}

export enum OperationType {
  Copy,
  Encrypt
}

export interface FileOperation {
  From: Path,
  To: Path,
  Operation: OperationType,
  Folder: FolderType
}