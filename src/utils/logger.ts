export const LogType = {
  INFO: "INFO",
  WARN: "WARN",
  ERROR: "ERROR",
  DEBUG: "DEBUG",
  SUCCESS: "SUCCESS",
} as const;

export type LogType = (typeof LogType)[keyof typeof LogType];

export type LogCategory = "USUAL" | "RENDER" | "BG";

export interface LogEntry {
  timestamp: string;
  filename: string;
  functionName: string;
  logType: LogType;
  category: LogCategory;
  message: string;
  stackTrace?: string;
}

class Logger {
  // Array of filenames (without path) that are allowed to log
  // Only logs from files in this array will be displayed
  // If empty, all files are allowed (backward compatibility)
  public allowedFiles: string[] = [];

  // Array of categories that are allowed to log
  // Only logs with categories in this array will be displayed
  // If empty, all categories are allowed (backward compatibility)
  public allowedCategories: LogCategory[] = [];

  private formatTimestamp(): string {
    return new Date().toISOString();
  }

  private formatLogEntry(entry: Omit<LogEntry, "timestamp">): string {
    const timestamp = this.formatTimestamp();
    const logTypeColor = this.getLogTypeColor(entry.logType);
    const resetColor = "\x1b[0m";

    return `${timestamp} [${logTypeColor}${entry.logType}${resetColor}] [${entry.category}] ${entry.filename}::${entry.functionName} - ${entry.message}`;
  }

  private getLogTypeColor(logType: LogType): string {
    switch (logType) {
      case LogType.ERROR:
        return "\x1b[31m"; // Red
      case LogType.WARN:
        return "\x1b[33m"; // Yellow
      case LogType.INFO:
        return "\x1b[36m"; // Cyan
      case LogType.DEBUG:
        return "\x1b[35m"; // Magenta
      case LogType.SUCCESS:
        return "\x1b[32m"; // Green
      default:
        return "\x1b[0m"; // Reset
    }
  }

  private logToConsole(entry: LogEntry): void {
    // Check filename filtering: if allowedFiles is set, filename must be in it
    if (
      this.allowedFiles.length > 0 &&
      !this.allowedFiles.includes(entry.filename)
    ) {
      return;
    }

    // Check category filtering: if allowedCategories is set, category must be in it
    if (
      this.allowedCategories.length > 0 &&
      !this.allowedCategories.includes(entry.category)
    ) {
      return;
    }

    const formattedMessage = this.formatLogEntry(entry);

    switch (entry.logType) {
      case LogType.ERROR:
        console.error(formattedMessage);
        if (entry.stackTrace) {
          console.error(entry.stackTrace);
        }
        break;
      case LogType.WARN:
        console.warn(formattedMessage);
        break;
      case LogType.DEBUG:
        // Use console.log to ensure DEBUG logs appear in all environments (renderer/main)
        console.log(formattedMessage);
        break;
      default:
        console.log(formattedMessage);
        break;
    }
  }

  private logToFile(entry: LogEntry): void {
    // Check filename filtering: if allowedFiles is set, filename must be in it
    if (
      this.allowedFiles.length > 0 &&
      !this.allowedFiles.includes(entry.filename)
    ) {
      return;
    }

    // Check category filtering: if allowedCategories is set, category must be in it
    if (
      this.allowedCategories.length > 0 &&
      !this.allowedCategories.includes(entry.category)
    ) {
      return;
    }

    // In a real application, you might want to write to a log file
    // For now, we'll just use console logging
    // This could be extended to write to a file using fs module
  }

  public log(
    filename: string,
    functionName: string,
    logType: LogType,
    message: string,
    error?: Error,
    category?: LogCategory
  ): void {
    // Extract just the filename from the path
    const extractedFilename = filename.replace(/^.*[\\\/]/, "");

    // Default to 'USUAL' if category is not provided
    const logCategory: LogCategory = category || "USUAL";

    const entry: LogEntry = {
      timestamp: this.formatTimestamp(),
      filename: extractedFilename,
      functionName,
      logType,
      category: logCategory,
      message,
      stackTrace: error?.stack,
    };

    this.logToConsole(entry);
    this.logToFile(entry);
  }

  // Convenience methods
  public info(
    filename: string,
    functionName: string,
    message: string,
    category?: LogCategory
  ): void {
    this.log(
      filename,
      functionName,
      LogType.INFO,
      message,
      undefined,
      category
    );
  }

  public warn(
    filename: string,
    functionName: string,
    message: string,
    category?: LogCategory
  ): void {
    this.log(
      filename,
      functionName,
      LogType.WARN,
      message,
      undefined,
      category
    );
  }

  public error(
    filename: string,
    functionName: string,
    message: string,
    error?: Error,
    category?: LogCategory
  ): void {
    this.log(filename, functionName, LogType.ERROR, message, error, category);
  }

  public debug(
    filename: string,
    functionName: string,
    message: string,
    category?: LogCategory
  ): void {
    this.log(
      filename,
      functionName,
      LogType.DEBUG,
      message,
      undefined,
      category
    );
  }

  public success(
    filename: string,
    functionName: string,
    message: string,
    category?: LogCategory
  ): void {
    this.log(
      filename,
      functionName,
      LogType.SUCCESS,
      message,
      undefined,
      category
    );
  }
}

// Export a singleton instance
export const logger = new Logger();

// Export the class for custom instances if needed
export { Logger };
