// Configuration module

export interface Config {
  makeDebug: boolean;
  makeTask: string;
  buildType: string;
  appName: string;
  forceFlag: boolean;
}

// Global configuration object
export const config: Config = {
  makeDebug: false,
  makeTask: "",
  buildType: "Release",
  appName: "MikoIDE",
  forceFlag: false
};

// Export individual properties for backward compatibility
export let makeDebug = config.makeDebug;
export let makeTask = config.makeTask;
export let buildType = config.buildType;
export let appName = config.appName;
export let forceFlag = config.forceFlag;

// Setters for updating configuration
export function setMakeDebug(value: boolean): void {
  config.makeDebug = value;
  makeDebug = value;
}

export function setMakeTask(value: string): void {
  config.makeTask = value;
  makeTask = value;
}

export function setBuildType(value: string): void {
  config.buildType = value;
  buildType = value;
}

export function setAppName(value: string): void {
  config.appName = value;
  appName = value;
}

export function setForceFlag(value: boolean): void {
  config.forceFlag = value;
  forceFlag = value;
}