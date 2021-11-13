export enum Level {
  SILENT = 0,
  DEBUG = 1,
  INFO = 2,
  WARNING = 3,
  ERROR = 4
}

function levelToString(level: Level): string {
  switch (level) {
  case Level.SILENT: return "SILENT";
  case Level.DEBUG: return "DEBUG";
  case Level.INFO: return "INFO";
  case Level.WARNING: return "WARNING";
  case Level.ERROR: return "ERROR";
  default: return "UNKNOWN";
  }
}

class Logger {
  minLevel: Level;

  constructor() {
    this.minLevel = Level.DEBUG;
  }

  private _log(level: Level, message: string) {
    if (level === Level.SILENT || this.minLevel === Level.SILENT) return;
    if (level < this.minLevel) return;

    const date = new Date();
    const logMessage = `${date.toISOString()}|${levelToString(level)}|${message}`;

    console.log(logMessage);
  }

  public log(message: string) {
    this._log(Level.INFO, message);
  }

  public debug(message: string) {
    this._log(Level.DEBUG, message);
  }

  public warn(message: string) {
    this._log(Level.WARNING, message);
  }

  public error(message: string) {
    this._log(Level.ERROR, message);
  }

  public setMinLevel(level: Level) {
    this.minLevel = level;
  }
}

const logger = new Logger();
export default logger;