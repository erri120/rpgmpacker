import chalk from "chalk";

export enum Level {
  SILENT = 0,
  DEBUG = 1,
  INFO = 2,
  WARNING = 3,
  ERROR = 4
}

class Logger {
  minLevel: Level;

  constructor() {
    this.minLevel = Level.DEBUG;
  }

  private _log(level: Level, message: string) {
    if (level === Level.SILENT || this.minLevel === Level.SILENT) return;
    if (level < this.minLevel) return;

    switch (level) {
    case Level.DEBUG:
      console.log(chalk.grey(message));
      break;
    case Level.INFO:
      console.log(message);
      break;
    case Level.WARNING:
      console.log(chalk.yellow(message));
      break;
    case Level.ERROR:
      console.log(chalk.red(message));
      break;
    default:
      break;
    }
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