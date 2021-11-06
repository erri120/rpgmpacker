import { createHash } from "crypto";
import fs from "fs";

export function getFileHash(file: string): string {
  const fileHash = createHash("SHA256");
  const contents = fs.readFileSync(file);
  fileHash.update(contents);
  return fileHash.digest("hex");
}