/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include <iostream>
#include <stdio.h>

#include "argtable2.h"

#include "base/core/str.h"
#include "base/core/assert.h"
#include "base/core/log_file.h"
#include "base/io/base_file.h"
#include "base/threading/thread.h"
#include "base/core/crash.h"

#include "log.h"
#include "plugin_manager.h"
#include "plugin_directory.h"
#include "config_parser.h"

#include <vector>
#include <string>
#include <signal.h>

bool g_running = true;
char g_working_dir[FILENAME_MAX];

void signal_handler(int sig) {
  LINK_INFO("signal cought: %d", sig);
  g_running = false;
}

void setup_logs() {
  Base::Log::LogFile general = Base::Log::CreateLogFileUnique("", "output");
  if(!general) {
    return;
  }
  Base::Log::LogFile release = Base::Log::CreateLogFileUnique("", "release");
  if(!release) {
    Base::Log::DestroyLogFile(general);
    return;
  }

  int release_filter =
      Base::Log::kLogWarning | Base::Log::kLogError | Base::Log::kLogCritical;

  Base::Log::AddFilter(general, Base::Log::kAnyCategory, Base::Log::kLogAll);
  Base::Log::AddFilter(release, Base::Log::kAnyCategory, release_filter);
}

int main(int argc, char *argv[]) {
  // disable stdout buffering for testing purposes.
  setbuf(stdout, nullptr);

  setup_logs();

  LINK_INFO("starting link server...");
  getcwd(g_working_dir, FILENAME_MAX);
  LINK_INFO("app path: %s", argv[0]);
  LINK_INFO("working dir: %s", g_working_dir);

  for(int i = 0; i < argc; ++i) {
    LINK_INFO("%d: %s", i, argv[i]);
  }

  Base::RegisterCrashHandler();

  if(SIG_ERR == signal(SIGINT, signal_handler)) {
    LINK_ERROR("failed to register signal handler for SIGINT");
    return -1;
  }

  if(SIG_ERR == signal(SIGTERM, signal_handler)) {
    LINK_ERROR("failed to register signal handler for SIGTERM");
    return -1;
  }

  struct arg_str *config_file =
      arg_str0("f", "file", "<file>", "configuration file");
  struct arg_str *config_data =
      arg_str0("c", "config", "<config>", "configuration data");
  struct arg_end *end = arg_end(20);
  void *arg_table[] = {config_file, config_data, end};

  if(arg_nullcheck(arg_table) != 0) {
    LINK_CRITICAL("could not parse arguments");
    return -2;
  }

  int nerrors = arg_parse(argc, argv, arg_table);

  if(nerrors > 0) {
    LINK_CRITICAL("%d error(s) occured during argument parsing", nerrors);
    arg_print_errors(stdout, end, "link server");
    arg_print_syntaxv(stdout, arg_table, "\n");
    arg_print_glossary(stdout, arg_table, " %-25s %s\n");
    return -3;
  }

  if(config_data->count == 0 && config_file->count == 0) {
    LINK_CRITICAL("no server config");
    return -4;
  }

  Link::ConfigParser config;

  if(config_file->count > 0) {
    Base::FileHandle file = Base::Open(*config_file->sval, Base::OM_Read);
    if(file < 0) {
      LINK_CRITICAL("cannot open config file. path: '%s'", *config_file->sval);
      return -5;
    }

    size_t file_size = Base::Size(file);
    const size_t kMaxConfigSize = 5 * 1024 * 1024;
    if(file_size > kMaxConfigSize) {
      LINK_CRITICAL("unsupported config file size: %dbytes", file_size);
      return -6;
    }

    s8 *buffer = new s8[file_size];
    size_t read = Base::Read(file, buffer, file_size);
    if(read != file_size) {
      LINK_CRITICAL("problem reading config file '%s'", *config_file->sval);
      delete[] buffer;
      return -7;
    }
    if(!config.FromData(buffer, read)) {
      LINK_CRITICAL("file config corrupted");
      delete[] buffer;
      return -8;
    }
    delete[] buffer;
  } else if(config_data) {
    if(!config.FromData(*config_data->sval,
                        Base::String::strlen(*config_data->sval))) {
      LINK_CRITICAL("config corrupted");
      return -9;
    }
  }

  // Base::Crash();

  Link::PluginDirectory directory;
  Link::PluginManager plugin_manager(directory);

  // free command arguments.
  arg_freetable(arg_table, sizeof(arg_table) / sizeof(arg_table[0]));

  for(int i = 0; i < config.GetPluginCount(); ++i) {
    // load plugin with configuration.
    const char *plugin_path = config.GetPluginPath(i);

    streamsize config_length = config.GetConfig(i, nullptr, 0);
    char *config_data = nullptr;

    if(config_length > 0) {
      config_data = new char[config_length];
      streamsize data_copied = config.GetConfig(i, config_data, config_length);
      BASE_ASSERT(data_copied == config_length);
    }

    plugin_manager.Load(plugin_path, config_data, config_length);
    delete[] config_data;
  }

  LinkConfiguration configuration;
  const char *hostname = config.GetHostname();
  if(!hostname) {
    LINK_WARN("no link hostname specified - defaulting to 127.0.0.1");
    hostname = "127.0.0.1";
  }

  Base::String::strncpy(configuration.hostname, hostname, kLinkHostnameMax);
  plugin_manager.StartAll(configuration);

  int runtime = config.GetRuntime();

  LINK_INFO("link server started. runtime: %d", runtime);

  if(runtime > 0) {
    // timed server run.
    while(g_running && runtime > 0) {
      unsigned int update = 100;
      Base::Thread::Sleep(update);
      runtime -= update;
    }
  } else {
    // infinite run.
    while(g_running) {
      Base::Thread::Sleep(1000);
    }
  }

  plugin_manager.StopAll();

  plugin_manager.UnloadAll();

  LINK_INFO("link server stopped.");
  return 0;
}
