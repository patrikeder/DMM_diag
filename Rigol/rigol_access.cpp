/*
 *  Command-Line Interface to RIGOL DS2000 VISA Interface
 *
 *  Copyright (C) 2013  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <visa.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <set>

extern const char *all_commands_raw[];
std::set<std::string> all_cmds;

//#define dbg_print printf
#define dbg_print //


RIGOL_access {
    void add_command(std::string txt)
    {
#if 0
        for (size_t i = 0; i < txt.size(); i++)
            if ('a' <= txt[i] && txt[i] <= 'z') {
                size_t k = i;
                while (i < txt.size() && 'a' <= txt[i] && txt[i] <= 'z')
                    i++;
                add_command(txt.substr(0, k) + txt.substr(i));
            }
#endif

        for (size_t i = 0; i < txt.size(); i++)
            if ('a' <= txt[i] && txt[i] <= 'z')
                txt[i] -= 'a' - 'A';

        all_cmds.insert(txt);
    }

    void flush(ViSession &vi)
    {
        char buffer[1024];

        while (1) {
            ViUInt32 count;
            if (viRead(vi, reinterpret_cast<ViPBuf>(buffer), sizeof(buffer), &count) != VI_SUCCESS) {
                fprintf(stderr, "Cannot read from resource!\n");
                exit(1);
            }
            if (count <= 0)
                break;
        }
    }

    void send_cmd(ViSession &vi, std::string cmd)
    {
        ViUInt32 count;
        if (viWrite(vi, reinterpret_cast<ViPBuf>(const_cast<char*>(cmd.data())), cmd.size(), &count) != VI_SUCCESS) {
            fprintf(stderr, "Cannot write to resource!\n");
            exit(1);
        }
    }

    unsigned char *recv_data(ViSession &vi, size_t len)
    {
        unsigned char *data = (unsigned char*)malloc(len+1);
        unsigned char *p = data;

        while (p != data+len) {
            ViUInt32 count;
            if (viRead(vi, reinterpret_cast<ViPBuf>(p), data+len-p, &count) != VI_SUCCESS) {
                fprintf(stderr, "Cannot read from resource (after %d bytes of data)!\n", int(p-data));
                exit(1);
            }
            p += count;
        }

        data[len] = 0;
        return data;
    }

    std::string recv_text(ViSession &vi)
    {
        int data_size = 1024, data_pos = 0;
        char *data = (char*)malloc(data_size);
        int timeout = 0;

        while (1) {
            ViUInt32 count;
            if (viRead(vi, reinterpret_cast<ViPBuf>(data+data_pos), data_size-data_pos, &count) != VI_SUCCESS) {
                fprintf(stderr, "Cannot read from resource (after %d bytes of text)!\n", data_pos);
                exit(1);
            }
            if (count == 0) {
                if (++timeout >= 2) {
                    dbg_print("TIMEOUT! -- Invalid query or missing query argument?\n");
                    break;
                }
            } else
                timeout = 0;
            data_pos += count;
            if (data[data_pos-1] == '\r' || data[data_pos-1] == '\n')
                break;
            if (data_pos > data_size/2) {
                data_size *= 2;
                data = (char*)realloc(data, data_size);
            }
        }

        while (data_pos > 0 && (data[data_pos-1] == '\r' || data[data_pos-1] == '\n' || data[data_pos-1] == 0))
            data_pos--;
        std::string text = std::string(data, data_pos);
        free(data);
        return text;
    }

    char *readline_cmd_generator(const char *text, int state)
    {
        static std::set<std::string>::iterator it;
        static std::string prefix;

        if (!state) {
            it = all_cmds.begin(), prefix = text;
            for (size_t i = 0; i < prefix.size(); i++)
                if ('a' <= prefix[i] && prefix[i] <= 'z')
                    prefix[i] -= 'a' - 'A';
        }

        for (; it != all_cmds.end(); it++) {
            std::string pf = it->substr(0, prefix.size());
            if (pf == prefix)
                return strdup((it++)->c_str());
        }

        return NULL;
    }

    char **readline_completion(const char *text, int start, int)
    {
        if (start == 0)
            return rl_completion_matches(text, this->readline_cmd_generator);
        return NULL;
    }

    bool run_command(ViSession &vi, char *serial, char *command)
    {
        std::string orig_command = command;
        char *cmd = strtok(command, " \t\r\n");

        if (cmd == NULL || cmd[0] == 0 || cmd[0] == '#')
            return true;

        add_history(orig_command.c_str());

        if (cmd[0] == '*' || cmd[0] == ':')
        {
            send_cmd(vi, orig_command);

            for (char *p = cmd; *p; p++)
                if ('a' <= *p && *p <= 'z')
                    *p -= 'a' - 'A';

            if (strlen(cmd) > 6 && cmd[strlen(cmd)-6] == ':' && cmd[strlen(cmd)-5] == 'D' && cmd[strlen(cmd)-4] == 'A' &&
                    cmd[strlen(cmd)-3] == 'T' && cmd[strlen(cmd)-2] == 'A' && cmd[strlen(cmd)-1] == '?')
            {
                unsigned char *header = recv_data(vi, 11);
                dbg_print("Header: %s\n", header);

                dbg_print("Data:");
                int num_bytes = atoi((char*)header + 2);
                unsigned char *data = recv_data(vi, num_bytes + 1);
                for (int i = 0; i < num_bytes; i++)
                    dbg_print(" %d", (unsigned char)data[i]);
                dbg_print("\n\n");

                free(header);
                free(data);
            } else if (cmd[strlen(cmd)-1] == '?')
            {
                std::string result = recv_text(vi);
                if (!result.empty())
                    dbg_print("%s\n", result.c_str());
                dbg_print("\n");
            }

            return true;
        }

        for (char *p = cmd; *p; p++)
            if ('a' <= *p && *p <= 'z')
                *p -= 'a' - 'A';

        if (!strcmp(cmd, "SLEEP"))
        {
            char *seconds = strtok(NULL, " \t\r\n");
            sleep(atoi(seconds));
            return true;
        }

        if (!strcmp(cmd, "WAIT"))
        {
            std::string status;
            dbg_print("Waiting for trigger..");
            fflush(stdout);
            sleep(1);

            while (1) {
                send_cmd(vi, ":TRIGGER:STATUS?");
                status = recv_text(vi);
                if (status != "WAIT")
                    break;
                dbg_print(".");
                fflush(stdout);
            }

            dbg_print(" %s\n\n", status.c_str());
            return true;
        }

        if (!strcmp(cmd, "SAVE_TXT") || !strcmp(cmd, "SAVE_BIN"))
        {
            char *query = strtok(NULL, " \t\r\n");
            char *filename = strtok(NULL, " \t\r\n");

            for (char *p = query; *p; p++)
                if ('a' <= *p && *p <= 'z')
                    *p -= 'a' - 'A';

            if (strlen(query) > 6 && query[strlen(query)-6] == ':' && query[strlen(query)-5] == 'D' && query[strlen(query)-4] == 'A' &&
                    query[strlen(query)-3] == 'T' && query[strlen(query)-2] == 'A' && query[strlen(query)-1] == '?')
            {
                send_cmd(vi, query);

                unsigned char *header = recv_data(vi, 11);
                dbg_print("Header: %s\n", header);

                int num_bytes = atoi((char*)header + 2);
                unsigned char *data = recv_data(vi, num_bytes + 1);

                FILE *f = fopen(filename, "w");
                if (f == NULL) {
                    fprintf(stderr, "Cannot open file '%s' for writing!\n", filename);
                    exit(1);
                }
                if (!strcmp(cmd, "SAVE_TXT")) {
                    for (int i = 0; i < num_bytes; i++)
                        fprintf(f, "%d\n", (unsigned char)data[i]);
                } else
                    fwrite(data, num_bytes, 1, f);
                fclose(f);

                dbg_print("Written %d bytes to file (%s).\n", num_bytes, !strcmp(cmd, "SAVE_TXT") ? "ascii" : "binary");

                free(header);
                free(data);
            } else if (query[strlen(query)-1] == '?')
            {
                send_cmd(vi, query);
                std::string result = recv_text(vi);

                FILE *f = fopen(filename, "w");
                if (f == NULL) {
                    fprintf(stderr, "Cannot open file '%s' for writing!\n", filename);
                    exit(1);
                }
                fprintf(f, "%s\n", result.c_str());
                fclose(f);

                dbg_print("Written %d characters to file.\n", int(result.size()));
            } else
                dbg_print("SAVE_* not supported for non-query command '%s'.\n", query);

            dbg_print("\n");
            return true;
        }

        if (!strcmp(cmd, "DOWNLOAD_TXT") || !strcmp(cmd, "DOWNLOAD_BIN"))
        {
            char *filename = strtok(NULL, " \t\r\n");

            send_cmd(vi, ":WAVEFORM:RESET");
            send_cmd(vi, ":WAVEFORM:BEGIN");

            FILE *f = fopen(filename, "w");
            if (f == NULL) {
                fprintf(stderr, "Cannot open file '%s' for writing!\n", filename);
                exit(1);
            }

            int total_bytes = 0;
            dbg_print("Downloading..");
            fflush(stdout);

            while (1)
            {
                send_cmd(vi, ":WAVEFORM:STATUS?");
                std::string status = recv_text(vi);

                send_cmd(vi, ":WAVEFORM:DATA?");
                unsigned char *header = recv_data(vi, 11);

                int num_bytes = atoi((char*)header + 2);
                unsigned char *data = recv_data(vi, num_bytes + 1);

                if (!strcmp(cmd, "DOWNLOAD_TXT")) {
                    for (int i = 0; i < num_bytes; i++)
                        fprintf(f, "%d\n", (unsigned char)data[i]);
                } else
                    fwrite(data, num_bytes, 1, f);

                total_bytes += num_bytes;
                dbg_print(".");
                fflush(stdout);

                free(header);
                free(data);

                if (status[0] == 'I') {
                    send_cmd(vi, ":WAVEFORM:END");
                    dbg_print("\n");
                    break;
                }
            }

            fclose(f);

            dbg_print("Written %d bytes to file (%s).\n\n", total_bytes, !strcmp(cmd, "DOWNLOAD_TXT") ? "ascii" : "binary");
            return true;
        }

        if (!strcmp(cmd, "INSTALL"))
        {
            std::string lic_key;
            char *opt = strtok(NULL, " \t\r\n");

            std::string sh_cmd = std::string("rigol-4in1-keygen ") + serial + " " + opt;
            char buffer[1024];

            FILE *p = popen(sh_cmd.c_str(), "r");
            while (fgets(buffer, 1024, p) != NULL) {
                if (!strncmp(buffer, "your-license-key: ", 18)) {
                    lic_key.clear();
                    for (int i = 0; i < 4*7+3; i++)
                        if (buffer[18+i] != '-')
                            lic_key += buffer[18+i];
                }
                dbg_print("*** %s", buffer);
            }
            pclose(p);

            if (lic_key.empty()) {
                fprintf(stderr, "Failed to generate licence key!\n");
                exit(1);
            }

            std::string install_cmd = ":SYSTEM:OPTION:INSTALL " + lic_key;
            dbg_print("> %s\n", install_cmd.c_str());

            send_cmd(vi, install_cmd);
            dbg_print("\n");
            return true;
        }

        if (!strcmp(cmd, "HELP"))
        {
            dbg_print("\n");
            dbg_print("    [:*]...\n");
            dbg_print("        send command to device and print results, e.g. '*IDN?'\n");
            dbg_print("        (see the DS2000 Series Programming Guide)\n");
            dbg_print("\n");
            dbg_print("    SLEEP <n>\n");
            dbg_print("        pause executeion for <n> seconds\n");
            dbg_print("\n");
            dbg_print("    WAIT\n");
            dbg_print("        wait for :TRIGGER:STATUS? to return something else than WAIT\n");
            dbg_print("\n");
            dbg_print("    SAVE_TXT <QUERY> <FILENAME>\n");
            dbg_print("        e.g. 'SAVE_TXT :WAV:DATA? wavedata.txt'\n");
            dbg_print("\n");
            dbg_print("    SAVE_BIN <QUERY> <FILENAME>\n");
            dbg_print("        e.g. 'SAVE_BIN :DISP:DATA? screenshot.bmp'\n");
            dbg_print("\n");
            dbg_print("    DOWNLOAD_TXT <FILENAME>\n");
            dbg_print("    DOWNLOAD_BIN <FILENAME>\n");
            dbg_print("        save large waveforms (use instead of SAVE_* in RAW mode)\n");
            dbg_print("        (see :WAVEFORM:DATA? in Programming Guide)\n");
            dbg_print("\n");
            dbg_print("    INSTALLL <option>\n");
            dbg_print("        generate key and install option, e.g. 'INSTALL DSA9'\n");
            dbg_print("        DSA? for permanent options, VSA? for temporary options\n");
            dbg_print("        this command needs 'rigol-4in1-keygen' in path\n");
            dbg_print("\n");
            dbg_print("    HELP\n");
            dbg_print("        print this help message\n");
            dbg_print("\n");
            dbg_print("    EXIT\n");
            dbg_print("        quit this program\n");
            dbg_print("\n");
            return true;
        }

        if (!strcmp(cmd, "EXIT"))
            return false;

        dbg_print("Unknown command '%s'. Type 'help' for help.\n\n", command);
        return true;
    }

    ~RIGOL_access()
    {
      
    }
    

    RIGOL_access(int argc, char **argv)
    {
        if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
            		dbg_print("\n");
            		dbg_print("Usage: %s VISA-RESOURCE [commands]\n", argv[0]);
            		dbg_print("       %s --scan\n", argv[0]);
            		dbg_print("\n");
            		dbg_print("E.g.: %s TCPIP::192.168.0.123::INSTR ':CALIBRATE:DATE?'\n", argv[0]);
            		dbg_print("\n");
            
            return(argc != 2);
        }

        ViSession rmgr;

        if (viOpenDefaultRM(&rmgr) != VI_SUCCESS) {
            fprintf(stderr, "Cannot open default resource manager!\n");
            exit(1);
        }

        if (!strcmp(argv[1], "--scan")) {
            ViFindList fl;
            ViUInt32 count;
            ViChar rsrc[256];
            ViStatus rc = viFindRsrc(rmgr, const_cast<ViChar*>("?*"), &fl, &count, rsrc);
            while (rc == VI_SUCCESS) {
                dbg_print("%s\n", rsrc);
                rc = viFindNext(fl, rsrc);
            }
            viClose(fl);
            viClose(rmgr);
            exit(0);
        }

        ViSession vi;

        if (viOpen(rmgr, argv[1], VI_NO_LOCK, 0, &vi) != VI_SUCCESS) {
            fprintf(stderr, "Cannot open resource %s!\n", argv[1]);
            exit(1);
        }

        flush(vi);
        send_cmd(vi, "*IDN?");
        std::string idn = recv_text(vi);

        char *serial_idn = strdup(idn.c_str());
        char *serial = strtok(serial_idn, ",");
        serial = strtok(NULL, ",");
        serial = strtok(NULL, ",");

        for (int i = 0; all_commands_raw[i]; i++)
            add_command(all_commands_raw[i]);
        add_command("SLEEP");
        add_command("WAIT");
        add_command("SAVE_TXT");
        add_command("SAVE_BIN");
        add_command("DOWNLOAD_TXT");
        add_command("DOWNLOAD_BIN");
        add_command("INSTALL");
        add_command("QUERY");
        add_command("HELP");
        add_command("EXIT");

        if (argc == 2)
        {
            dbg_print("\nConnected to %s.\n", idn.c_str());
            dbg_print("Type 'help' for help.\n\n");

            char prompt[1024];
            snprintf(prompt, 1024, "%s> ", serial);

            rl_attempted_completion_function = readline_completion;
            rl_basic_word_break_characters = " \t\n";

            char *command = NULL;
            while ((command = readline(prompt)) != NULL)
                if (!run_command(vi, serial, command))
                    break;
            if (command == NULL)
                dbg_print("EXIT\n");
        }
        else
        {
            dbg_print("\nConnected to %s.\n\n", idn.c_str());
            for (int i = 2; i < argc; i++) {
                dbg_print("%s> %s\n", serial, argv[i]);
                if (!run_command(vi, serial, argv[i]))
                    break;
            }
        }

        viClose(vi);
        viClose(rmgr);

        return 0;
    }

 };
