/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Terminal.h"

#include <FreeRTOS.h>
#include <task.h>

#include <string.h>

#include "FormattedIO.h"
#include "Ptr.h"
#include "Task.h"

/*
 * Commands
 */

static int CommandEcho(char* aArguments)
{
  if (!aArguments) {
    aArguments = ""; /* set an empty string */
  }
  int res = Print("%s\n\r", aArguments);
  if (res < 0) {
    return -1;
  }
  return 0;
}

typedef struct
{
  char mName[8];
  int (*mHandle)(char* mArguments);
} Command;

static const Command sCommands[] = {
  { "echo", CommandEcho }
};

static const Command*
FindCommand(const char* aName)
{
  const Command* end = sCommands + ArrayLength(sCommands);
  for (const Command* cmd = sCommands; cmd < end; ++cmd) {
    if (!strcmp(aName, cmd->mName)) {
      return cmd;
    }
  }
  return NULL;
}

/*
 * Task
 */

typedef struct
{
  TaskHandle_t mTask;
} TerminalTask;

static void
RunTask(TerminalTask* aTerminal)
{
  for (;;) {
    Print("> ");

    /* read input */
    char buf[128];
    ssize_t res = Getline(buf, sizeof(buf));
    if (res < 0) {
      continue;
    }

    /* input processing */
    size_t off = strspn(buf, " \t"); /* filter out trailing whitespaces */
    char* pos = buf + off;
    const char* name = strsep(&pos, " \t\n\r");
    if (!name) {
      continue;
    }
    const Command* cmd = FindCommand(name);
    if (!cmd) {
      Print("%s: %s: unknown command\n\r", "terminal", name);
      continue;
    }
    off = strspn(pos, " \t"); /* filter out whitespaces */
    char* args = pos + off;

    /* execute command */
    cmd->mHandle(args);
  }
}

static void
TaskEntryPoint(void* aParam)
{
  TerminalTask* terminal = aParam;

  RunTask(terminal);

  /* We mark ourselves for deletion. Deletion is done by
   * the idle thread. We suspend until this happens. */
  vTaskDelete(terminal->mTask);
  vTaskSuspend(terminal->mTask);
}

static int
TerminalTaskInit(TerminalTask* aTerminal)
{
  aTerminal->mTask = NULL;

  return 0;
}

static int
TerminalTaskSpawn(TerminalTask* aTerminal)
{
  BaseType_t res = xTaskCreate(TaskEntryPoint, "terminal",
                               TaskDefaultStackSize(), aTerminal,
                               1, &aTerminal->mTask);
  if (res != pdPASS) {
    return -1;
  }
  return 0;
}

/*
 * Public interface
 */

static TerminalTask sTerminalTask;

int
TerminalInit()
{
  if (TerminalTaskInit(&sTerminalTask) < 0) {
    return -1;
  }
  if (TerminalTaskSpawn(&sTerminalTask) < 0) {
    return -1;
  }

  return 0;
}
