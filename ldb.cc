#include "ldb.h"

namespace ldb {

  cDef get { GET, "get", "g" };
  cDef put { PUT, "put", "p" };
  cDef del { DEL, "del", "d" };
  cDef ls { LS, "ls" };
  cDef start { START, "start", "gt" };
  cDef end { END, "end", "lt" };
  cDef limit { LIMIT, "limit", "l" };

  vector<cDef> cmds = {
    get,
    put,
    del,
    ls,
    start,
    end,
    limit
  };

  vector<string> key_cache;
}

int main(int argc, char** argv)
{
  string path;
  string history_file = ".ldb_history";

  if (argc == 2) path = argv[1];

  string key_start = "";
  string key_end = "~";
  int key_limit = 1000;

  leveldb::DB* db;
  leveldb::Options options;

  int c;

  while((c = getopt(argc, argv, "i:ec:d")) != EOF)
  {
    switch (c)
    {
      case 'i':
        path = optarg;
      break;
      case 'e':
        options.error_if_exists = true;
      break;
      case 'c':
        path = optarg;
        options.create_if_missing = true;
      break;
    }
  }

  leveldb::Status status = leveldb::DB::Open(options, path, &db);

  if (false == status.ok())
  {
    cerr << "Unable to open/create database './testdb'" << endl;
    cerr << status.ToString() << endl;
    return -1;
  }

  char *line = NULL;
  static int quit = 0;

  linenoiseSetCompletionCallback(ldb::auto_completion);
  linenoiseHistoryLoad(history_file.c_str());

  ldb::range(
    db,
    key_start,
    key_end,
    ldb::key_cache,
    true);

  while ((line = linenoise("> "))) {

    if ('\0' == line[0]) cout << endl;

    string l = line;
    ldb::command cmd = ldb::parse_cmd(l, ldb::cmds);

    switch (cmd.id)
    {
      case GET:
      {
        ldb::get_value(db, cmd);
        break;
      }

      case PUT:
      {
        ldb::put_value(db, cmd);
        break;
      }

      case DEL:
      {
        break;
      }

      case LS:
      {

        ldb::range(
          db,
          key_start,
          key_end,
          ldb::key_cache,
          false);

        break;
      }

      case START:
      {
        cout << "START set to " << cmd.rest << endl;
        key_start = cmd.rest;
        break;
      }

      case END:
      {
        cout << "END set to " << cmd.rest << endl;
        key_end = cmd.rest;
        break;
      }

      case LIMIT:
      {
        string msg = "LIMIT set to ";

        if (cmd.rest.length() == 0) {
          cout << msg << key_limit << endl;
          break;
        }
        cout << msg << cmd.rest << endl;
        key_limit = atoi(cmd.rest.c_str());
        break;
      }

      default: {
        cout << l << endl;
      }
    }

    linenoiseHistoryAdd(line);
    linenoiseHistorySave(history_file.c_str());

    free(line);
  }

   delete db;
}


