#include <iostream>
#include <fstream>
#include <getopt.h>
#include <dirent.h>
#include <unordered_map>
#include <queue>

using namespace std;

const char sep =
#if defined _WIN32 || defined __CYGWIN__
    '\\';
#else
    '/';
#endif
const char seps[2] = {sep, '\0'};

int get_conf_map(char *conf_file, unordered_map<string, bool> &hm){
	string line_str;
	ifstream fin(conf_file);
	if (!fin){
		cerr << "Cann't open file " << conf_file << endl;
		return 1;
	}
	while (getline(fin, line_str)){
		line_str = line_str.substr(0, line_str.find_first_of('#'));
		line_str.erase(0, line_str.find_first_not_of(' '));
		line_str.erase(line_str.find_last_not_of(' ') + 1);
		line_str.erase(0, line_str.find_first_not_of(sep));
		line_str.erase(line_str.find_last_not_of(sep) + 1);
		if (line_str.length() == 0) continue;

		string::size_type last_pos = 0;
		string::size_type sep_pos = line_str.find(sep);
		while (true){
			if (sep_pos != string::npos){
				cout << "hm["<<line_str.substr(0, sep_pos)<<"] = false"<< endl;
				hm[line_str.substr(0, sep_pos)] = false;
			} else {
				cout << "hm["<<line_str.substr(0, sep_pos)<<"] = true"<< endl;
				hm[line_str.substr(0, sep_pos)] = true;
				break;
			}
			last_pos = sep_pos + 1;
			sep_pos = line_str.find(sep, last_pos);
		}
	}
	return 0;
}

int show_non_whitelist_paths(char *dir,  unordered_map<string, bool> &hm){
	queue<string> q_dir;
	q_dir.push("");
	while (!q_dir.empty()){
		string c_dir = string(dir) + sep + q_dir.front();
		DIR *dp = opendir(c_dir.c_str());
		if (dp == nullptr) {
			cerr << "Cann't open directory " << c_dir << endl;
			q_dir.pop();
			continue;
		}

		struct dirent *entry;
		while (entry = readdir(dp)){
			string new_sub_dir(entry->d_name);
			if (new_sub_dir == "." || new_sub_dir == "..") continue;

			if (q_dir.front().length() != 0)
				new_sub_dir = q_dir.front() + sep + entry->d_name;

			if (hm.find(new_sub_dir) == hm.end()){
				cout << new_sub_dir << endl;
				continue;
			}

			if (!hm[new_sub_dir]){
				if (entry->d_type == DT_DIR && entry->d_type != DT_LNK){
					cout << "push: "<<new_sub_dir << endl;
					q_dir.push(new_sub_dir);
				}
			}
		}
		closedir(dp);
		q_dir.pop();
	}
	return 0;
}

void print_help(){
	static const char *help_msg = 
		"Usage: $0 [-c <file>] [-d <directory>]\n"
		"List files not in the whitelist.\n"
		"\n"
		"       -c --config    whitelist file\n"
		"       -d --dir       Destination folder\n";
	cout << help_msg;
}

int main(int argc, char *argv[]){
	char *config = nullptr, *dir = nullptr;
	static struct option long_options[] = {
	/*   NAME       ARGUMENT           FLAG  SHORTNAME */
		{"config",  required_argument, NULL, 'c'},
		{"dir",     required_argument, NULL, 'd'},
		{"help",    no_argument,       NULL, 'h'},
		{NULL,      0,                 NULL, 0}
	};
	int option_index = 0;
	int c;
	while ((c = getopt_long(argc, argv, "c:d:h",
				 long_options, &option_index)) != -1) {
		int this_option_optind = optind ? optind : 1;
		switch (c) {
		case 'c':
			config = optarg;
			break;
		case 'd':
			dir = optarg;
			break;
		case 'h':
			print_help();
			return 0;
		case '?':
			cout << "?" <<endl;
			break;
		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (config == nullptr || dir == nullptr){
		print_help();
		return 1;
	}

	unordered_map<string, bool> hm;
	int ret = get_conf_map(config, hm);
	if (ret) return ret;

	ret = show_non_whitelist_paths(dir, hm);
	if (ret) return ret;
	return 0;
}

