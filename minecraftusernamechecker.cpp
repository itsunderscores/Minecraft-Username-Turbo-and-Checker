#pragma warning( disable : 4996)
#define CURL_STATICLIB 
#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <filesystem>
#include <regex>
#include "json.hpp"
#include <thread>
using json = nlohmann::json;

using ::std::array;
using ::std::cout;
using ::std::endl;
using ::std::regex;
using ::std::ref;
#define MAX_THREADS 200
using namespace std;

//For CURL, ignore
size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}


//List of UserAgents
string user_agent()
{
	srand(time(0));
	string choices[] = {
		"Mozilla/5.0 (Macintosh; PPC Mac OS X 10_6_8) AppleWebKit/5360 (KHTML, like Gecko) Chrome/36.0.811.0 Mobile Safari/5360",
		"Mozilla/5.0 (X11; Linux i686) AppleWebKit/5332 (KHTML, like Gecko) Chrome/37.0.859.0 Mobile Safari/5332",
		"Opera/9.28 (X11; Linux i686; en-US) Presto/2.8.300 Version/12.00",
		"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/5321 (KHTML, like Gecko) Chrome/40.0.831.0 Mobile Safari/5321",
		"Mozilla/5.0 (Windows NT 5.1) AppleWebKit/5321 (KHTML, like Gecko) Chrome/37.0.853.0 Mobile Safari/5321",
		"Opera/8.67 (X11; Linux i686; en-US) Presto/2.8.225 Version/11.00",
		"Opera/8.77 (Windows CE; en-US) Presto/2.12.326 Version/10.00",
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:90.0) Gecko/20100101 Firefox/90.0",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 11_5_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) FxiOS/35.0 Mobile/15E148 Safari/605.1.15",
		"Mozilla/5.0 (iPad; CPU OS 11_5_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) FxiOS/35.0 Mobile/15E148 Safari/605.1.15",
		"Mozilla/5.0 (iPod touch; CPU iPhone OS 11_5_1 like Mac OS X) AppleWebKit/604.5.6 (KHTML, like Gecko) FxiOS/35.0 Mobile/15E148 Safari/605.1.15",
		"Mozilla/5.0 (Linux; Android 8.0.0; SM-G960F Build/R16NW) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.84 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; Android 7.0; SM-G892A Build/NRD90M; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/60.0.3112.107 Mobile Safari/537.36",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) CriOS/69.0.3497.105 Mobile/15E148 Safari/605.1",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) FxiOS/13.2b11866 Mobile/16A366 Safari/605.1.15",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.34 (KHTML, like Gecko) Version/11.0 Mobile/15A5341f Safari/604.1",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A5370a Safari/604.1"
	};
	string random;
	for (int i = 0; i < 3; i++)
	{
		random = choices[rand() % 5];
	}
	return random;
}


//Check Account Status (Availbility)
string checkusername(string username, string auth) {
	auto curl = curl_easy_init();
	std::string response_string;
	std::string header_string;
	try {
		string url = "https://api.minecraftservices.com/minecraft/profile/name/" + username + "/available";
		if (curl) {

			struct curl_slist* slist1;
			slist1 = NULL;
			slist1 = curl_slist_append(slist1, "Content-Type: application/json");

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
			curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, auth);
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, "8000");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

			char* url;
			double elapsed;
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
			curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl = NULL;
		}

		try {
			nlohmann::json jsonData = nlohmann::json::parse(response_string);
			if (jsonData["status"] == "AVAILABLE") {
				return "AVAILABLE";
			}
			else {
				if (jsonData["status"] == "DUPLICATE") {

					return "TAKEN";
				}
				else {
					if (jsonData["error"] == "CONSTRAINT_VIOLATION") {
						return "CONSTRAINT_VIOLATION";
					}
					else {
						return jsonData["error"];
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			return "UNKOWN";
		}

	}
	catch (const std::exception& e)
	{
		return "UNKOWN";
	}
}

//Login to Minecraft Account (Get Auth)
string mclogin(string username, string password) {
	auto curl = curl_easy_init();
	std::string response_string;
	std::string header_string;

	std::string jsonstr = "{\"agent\": {\"name\": \"Minecraft\", \"version\": 1}, \"username\": \"" + username + "\", \"password\": \"" + password + "\", \"requestUser\": true}";

	try {
		string url = "https://authserver.mojang.com/authenticate";
		if (curl) {

			struct curl_slist* slist1;
			slist1 = NULL;
			slist1 = curl_slist_append(slist1, "Content-Type: application/json");

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, "8000");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

			char* url;
			double elapsed;
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
			curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl = NULL;

			try {
				nlohmann::json jsonData = nlohmann::json::parse(response_string);
				string key = jsonData["accessToken"];
				return key;
			}
			catch (const std::exception& e)
			{
				return "UNKOWN";
			}
		}
	}
	catch (const std::exception& e)
	{
		return "UNKOWN";
	}
}

//Snipe Account
string mcsnipe(string username, string auth) {
	auto curl = curl_easy_init();
	std::string response_string;
	std::string header_string;
	try {
		string url = "https://api.minecraftservices.com/minecraft/profile/name/" + username;
		if (curl) {

			struct curl_slist* slist1;
			slist1 = NULL;
			//slist1 = curl_slist_append(slist1, "Content-Type: application/json");

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
			//if (proxy != "0") { curl_easy_setopt(curl, CURLOPT_PROXY, proxy); }
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent());
			curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, auth);
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
			curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, "8000");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

			char* url;
			double elapsed;
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
			curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl = NULL;
		}
		try {
			nlohmann::json jsonData = nlohmann::json::parse(response_string);
			if (jsonData["name"] == username) {
				return "SNIPED";
			}
			else {
				if (jsonData["details"]["status"] == "DUPLICATE") {
					return "DUPLICATE";
				}
				else {
					if (jsonData["path"] == "/minecraft/profile/name/" + username) {
						return "LIMITED";
					}
					else {
						return "UNKNOWN";
						//cout << "[-] Unkown Response: " << response_string << "\n";
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			return "UNKNOWN";
		}
	}
	catch (const std::exception& e)
	{
		return "UNKNOWN";
	}
}


int main()
{

	string option;
	cout << "[+] Select option...\n";
	cout << "[-] 1 = Turbo from list\n";
	cout << "[-] 2 = Check usernames from list";
	cin >> option;

	string delay;
	cout << "\n[+] Enter delay per check (in seconds): ";
	cin >> delay;
	int delay1 = stoi(delay);

	cout << "\n\n[+] Starting...";

	//Grabs accounts from textfile to turbo
	if (option == "1") {

		ifstream file1("accounts.txt");
		string content1;
		while (file1 >> content1) {

			int error = 1;

			//Delimeter Split
			string account = content1;
			std::string delimiter = ":";
			std::string email = account.substr(0, account.find(delimiter[0]));
			std::string password = account.substr(int(email.length()), account.find(delimiter[0]));
			password = std::regex_replace(password, std::regex(":"), "");

			//Login to account
			string token = mclogin(email, password);
			if (token != "UNKOWN") {
				//Successful login
				cout << "[+] Signed into " << account << "\n";

				//Grab username list
				ifstream file("usernames.txt");
				string content;
				while (file >> content) {

					//Check username if available
					string result = checkusername(content, token);

					if (error >= 3) {
						cout << "[-] Got unkown response from request... trying new account!\n";
						break;
					}

					if (result == "UNKNOWN") {
						cout << "[-] Got unkown response from request... sleeping for 20 seconds!\n";
						this_thread::sleep_for(chrono::seconds(20));
						error++;
					}

					if (result == "AVAILABLE") {

						std::ofstream log("1_available.txt", std::ios_base::app | std::ios_base::out);
						log << content << " is available!\n";

						string grab = mcsnipe(content, token);
						if (grab == "SNIPED") {
							cout << "[!] Successfully turboed " << content << " onto " << account << " - Getting another account to continue!";
							std::ofstream log("1_success.txt", std::ios_base::app | std::ios_base::out);
							log << content << " onto " << account << "\n";
							break;
						}
					}
					else {
						//Failed
						cout << "[-] " << content << " - " << result << "\n";

						std::ofstream log("1_failed.txt", std::ios_base::app | std::ios_base::out);
						log << content << " - " << result << "\n";

						this_thread::sleep_for(chrono::seconds(delay1));
					}
				}
			}
			else {
				cout << "[-] Account information was invalid: " << account << " - Trying next account...\n";
			}
			this_thread::sleep_for(chrono::seconds(10)); //Sleep 10 seconds before trying new account
		}
	}

	if (option == "2") {
		ifstream file1("accounts.txt");
		string content1;
		while (file1 >> content1) {

			int error = 1;

			//Delimeter Split
			string account = content1;
			std::string delimiter = ":";
			std::string email = account.substr(0, account.find(delimiter[0]));
			std::string password = account.substr(int(email.length()), account.find(delimiter[0]));
			password = std::regex_replace(password, std::regex(":"), "");

			//Login to account
			string token = mclogin(email, password);
			if (token != "UNKOWN") {

				//Successful signin
				cout << "[+] Signed into " << account << "\n";

				ifstream file("usernames.txt");
				string content;
				while (file >> content) {
					string result = checkusername(content, token);

					if (error >= 3) {
						cout << "[-] Got unkown response from request... trying new account!\n";
						break;
					}

					if (result == "UNKNOWN") {
						cout << "[-] Got unkown response from request... sleeping for 20 seconds!\n";
						this_thread::sleep_for(chrono::seconds(20));
						error++;
					}

					if (result == "AVAILABLE") {
						std::ofstream log("2_available.txt", std::ios_base::app | std::ios_base::out);
						log << content << " - " << result << "\n";
					}
					else {
						std::ofstream log("2_failed.txt", std::ios_base::app | std::ios_base::out);
						log << content << " - " << result << "\n";
					}
					cout << content << " -> " << result << "\n";
					this_thread::sleep_for(chrono::seconds(delay1));
				}
			}
			else {
				cout << "[-] Account information was invalid: " << account << " - Trying next account...\n";
			}
			this_thread::sleep_for(chrono::seconds(10)); //Sleep 10 seconds before trying new account
		}
	}

	cout << "[-] Finished\n";
	system("pause");
}