import std.datetime : SysTime;
import std.stdio : writeln, writefln;
import std.file : getTimes, rename, remove, copy;
import core.stdc.stdlib : exit;
import std.process : wait, spawnProcess;
import std.algorithm : any, map;
import std.getopt;

string TARGET = "bulb";
string[] SRC = ["main.d"];

SysTime getModificationTime(string fileName)
{
  SysTime accessTime;
  SysTime modificationTime;
  getTimes(fileName, accessTime, modificationTime);
  return modificationTime;
}

bool should_rebuild(string binary, string[] sources)
{
  try {
    SysTime binaryModTime = getModificationTime(binary);
    auto srcModTime = sources.map!(src => getModificationTime(src));

    return any!(a => a > binaryModTime)(srcModTime);
  }
  catch (Exception e) {
    return true;
  }
}

int run(string[] args)
{
  writeln("[CMD]: ", args);
  auto status = wait(spawnProcess(args));

  if (status)
    writeln("[ERROR]: exited abnormally with code ", status);
  return status;
}

int build()
{
  if (!should_rebuild(TARGET, SRC))
    return 0;
  return run(["rdmd", "--build-only", "-of=" ~ TARGET] ~ SRC);
}

int clean()
{
  try {
    remove(TARGET);
  } catch (Exception e) {}

  return 0;
}

int install()
{
  int status = build();

  if (status != 0)
    return status;

  try {
    copy(TARGET, "/usr/bin/" ~ TARGET);
  } catch (Exception e) {
    writeln(e.msg);
  }

  return 0;
}

int main(string[] args)
{
  if (args.length == 1)
    return build();

  foreach (arg; args[1..$]) {
    switch (arg) {
      case "install": install(); break;
      case "clean": clean(); break;
      case "build": build(); break;
      default: writefln("Unknown option '%s'", arg);
    }
  }

  return 0;
}
