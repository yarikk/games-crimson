/* mklocale -- compile or disassemble a language file for Crimson Fields
   Copyright (C) 2004-2007 Jens Granseuer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <fstream>
#include <string.h>

#include "SDL.h"

#include "lang.h"
#include "globals.h"

#ifdef _MSC_VER
// SDL_Main linkage destroys the command line in VS8
#undef main
#endif

class LanguageTemplate {
public:
  int Parse(const char *fname, Language &lang) const;
  int Write(const char *fname, const Language &lang) const;

private:
  int ParseMessages(ifstream &file, Language &lang) const;
  void RemWS(string &str) const;
};

static char parse_args(int argc, char *argv[]);

static const char *templatefile;
static const char *localefile;

/* an exemplary language template file might look like this:
 *
 * # foobar language file for Crimson Fields
 * # translated by Foo Bar
 * language=foobar
 * id=fo
 *
 * [messages]
 * first message
 * # <- message delimiter
 * second message
 * # multiple delimiters are possible
 * #
 * third message
 * containing a line break
 * [/messages]
 */
int LanguageTemplate::Parse(const char *fname, Language &lang) const {
  int rc = 0;

  ifstream file(fname);
  if (file.is_open()) {

    string buf, value;
    unsigned int line = 0;
    size_t i;

    while (!file.eof()) {
      getline(file, buf);
      ++line;

      RemWS(buf);
      if ((buf.size() > 0) && (buf[0] != '#')) { /* ignore comments and empty lines */
        if (strncasecmp(buf.c_str(), "language", 8) == 0) {
          i = buf.find("=", 8);
          if (i == string::npos) {
            cerr << "Error in line " << line << ": format invalid" << endl;
            rc = -1;
            break;
          }

          value = buf.substr(i+1, 50);
          RemWS(value);

          lang.SetName(value.c_str());
        } else if (strncasecmp(buf.c_str(), "id", 2) == 0) {
          i = buf.find("=", 2);
          if (i == string::npos) {
            cerr << "Error in line " << line << ": format invalid" << endl;
            rc = -1;
            break;
          }

          value = buf.substr(i+1, 10);
          RemWS(value);

          if (value.size() != 2) {
            cerr << "Error in line " << line << ": only two-character identifiers allowed" << endl;
            rc = -1;
            break;
          }

          lang.SetID(value.c_str());
        } else if (strncasecmp(buf.c_str(), "[messages]", 10) == 0) {
          rc = ParseMessages(file, lang);
        } else {
          rc = -1;
          cerr << "Error in line " << line << ": unkown token" << endl;
          break;
        }
      }
    }
    file.close();

    if (strlen(lang.ID()) == 0) {
      cerr << "Error parsing template: no language id found" << endl;
      rc = -1;
    } else if (strlen(lang.Name()) == 0) {
      cerr << "Error parsing template: no language name found" << endl;
      rc = -1;
    }
  } else
    cerr << "Error opening template file" << endl;

  return rc;
}

int LanguageTemplate::ParseMessages(ifstream &file, Language &lang) const {
  bool done = false;
  string buf, msg;

  do {
    getline(file, buf);
    RemWS(buf);

    if ((buf.size() > 0) && 
        ((buf[0] == '#') || (strncasecmp(buf.c_str(), "[/messages]", 11) == 0))) {
      /* save last message */
      if (!msg.empty()) {
        lang.AddMsg(msg);
        msg.erase();
      }

      if (strncasecmp(buf.c_str(), "[/messages]", 11) == 0)
        done = true;
    } else {
      if (!msg.empty()) msg += '\n';
      msg.append(buf);
    }
  } while ( !file.eof() && !done );

  int rc = 0;

  if (file.eof()) {
    rc = -1;
    cerr << "Error: messages section unterminated" << endl;
  }

  return rc;
}

/* remove leading and trailing spaces and cr + lf from the string */
void LanguageTemplate::RemWS(string &str) const {
  size_t i = 0;

  if (str.empty())
    return;

  while (str[i] == ' ') ++i;

  str.erase(0, i);

  for (i = str.size() - 1;
        (i >= 0) && ((str[i] == ' ') || (str[i] == '\n') ||
                     (str[i] == '\r')); --i);  /* empty loop */
  str.erase(i+1);
}

int LanguageTemplate::Write(const char *fname, const Language &lang) const {
  int rc;

  ofstream file(fname);
  if (file.is_open()) {

    file << "# " << lang.Name() << " language template for Crimson Fields " << VERSION << endl;
    file << "language=" << lang.Name() << endl;
    file << "id=" << lang.ID() << endl;
    file << endl;
    file << "[messages]" << endl;

    const char *msg = lang.GetMsg(0);
    file << msg << endl;

    for (int i = 1; (msg = lang.GetMsg(i)) != 0; ++i) {
      file << '#' << endl;
      file << msg << endl;
    }

    file << "[/messages]" << endl;

    file.close();
    rc = 0;
  } else {
    rc = -1;
    cerr << "Error opening template file" << endl;
  }
  return rc;
}


int main(int argc, char *argv[]) {

  if (SDL_Init(0) < 0) {
    cerr << "Couldn't init SDL:" << endl
              << SDL_GetError();
    exit(-1);
  }
  atexit(SDL_Quit);

  char op = parse_args(argc, argv);
  if (op == 0) return 1;

  Language lang;
  LanguageTemplate tmpl;
  int rc;

  if (op == 'l') {
    rc = tmpl.Parse(templatefile, lang);

    if (rc == 0) {

      rc = lang.WriteCatalog(localefile);
      if (rc == -1)
        cerr << "Error writing catalog file" << endl;
      else rc = 0;
    }

  } else {
    rc = lang.ReadCatalog(localefile);

    if (rc != -1) {

      rc = tmpl.Write(templatefile, lang);

    } else
      cerr << "Error reading catalog file" << endl;
  }

  return rc;
}

char parse_args(int argc, char *argv[]) {
  bool usage = true;
  char rc = 0;

  if (argc == 4) {
    string op(argv[1]);

    if (op == "-l") {
      rc = 'l';
      templatefile = argv[2];
      localefile = argv[3];
      usage = false;
    } else if (op == "-t") {
      rc = 't';
      templatefile = argv[3];
      localefile = argv[2];
      usage = false;
    }
  }

  if (usage) {
    cerr << "Usage:" << endl
              << argv[0] << " -l <template> <locale> (create locale)" << endl
              << argv[0] << " -t <locale> <template> (create template)" << endl;
  }

  return rc;
}

