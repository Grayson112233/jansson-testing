// Simple test of the Jansson JSON library
// Load GitHub commits for a given repository

#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <curl/curl.h>

#define URL_FORMAT "https://api.github.com/repos/%s/%s/commits"
#define URL_SIZE 500
// 256 KB for GET request buffer
#define BUFFER_SIZE 1025 * 256

struct write_result {
	char *data;
	int pos;
};

// CURL WRITEFUNCTION 
size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream) {
	// Cast pointer to proper type
	struct write_result *result = (struct write_result *) stream;
	// Check data size vs buffer size
	if(result->pos + size * nmemb >= BUFFER_SIZE - 1) {
		printf("Response too large for buffer\n");
		return 0;
	}
	// Copy data and return position
	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;
	return size * nmemb;
}

char *request(const char *url) {
	CURL *curl = NULL;
	CURLcode status;
	struct curl_slist *headers = NULL;
	char *data = NULL;
	long code;

	// INIT
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(!curl){
		printf("Error initalizing CURL\n");
		goto error;
	}
	data = malloc(BUFFER_SIZE);
	if(!data){
		printf("Unable to allocate memory for GET request\n");
		goto error;
	}
	struct write_result write_result = {
		.data = data,
		.pos = 0
	};

	// Set URL
	curl_easy_setopt(curl, CURLOPT_URL, url);

	// Set Headers
	// GitHub API requires a User-Agent header
	headers = curl_slist_append(headers, "User-Agent: Jansson-Test");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	
	// Set Write Fuction
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

	status = curl_easy_perform(curl);
	if(status != 0) {
		printf("Error: Unable to request commit data\n");
		goto error;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if(code != 200) {
		printf("Error: Server returned HTTP code %ld\n", code);
		goto error;
	}

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();

	// Make sure there's a null char at the end
	data[write_result.pos] = '\0';

	return data;

error:
	if(data) { free(data); }
	if(curl) { curl_easy_cleanup(curl); }
	if(headers) { curl_slist_free_all(headers); }
	curl_global_cleanup();
	return NULL;

}

// Return offset of first newline in given string
int newline_offset(const char *text){
	const char *newline = strchr(text, '\n');
	// Make sure a newline exists in the string
	if(!newline) {
		return strlen(text);
	} else {
		return (int)(newline - text);
	}
}

int main(int argc, char *argv[]) {
	
	// Make sure the arguments are correct
	if(argc != 3) {
		printf("Usage: %s <user> <repo>\n", argv[0]);
		return 2;
	}

	// Create URL
	char url[URL_SIZE];
	snprintf(url, URL_SIZE, URL_FORMAT, argv[1], argv[2]);

	// Make request
	char *text = request(url);
	if(!text){
		printf("Request to GitHub API failed.\n");
	}

	json_error_t error;
	json_t *root = json_loads(text, 0, &error);
	free(text);
	
	if(!root) {
		printf("Error on line %d: %s\n", error.line, error.text);
		return 1;
	}

	if(!json_is_array(root)) {
		printf("Error: Root json object is not an array.\n");
		// Free the json object
		json_decref(root);
		return 2;
	}

	for(int i = 0; i < json_array_size(root); i++) {
		json_t *data, *sha, *commit, *message;
		const char *message_text;
		data = json_array_get(root, i);
		// COMMIT DATA
		if(!json_is_object(data)) {
			printf("Error: Commit data is not an object\n");
			json_decref(root);
			return 1;
		}
		// SHA
		sha = json_object_get(data, "sha");
		if(!json_is_string(sha)) {
			printf("Error: Commit SHA is not a string\n");
			json_decref(root);
			return 1;
		}
		// COMMIT OBJECT
		commit = json_object_get(data, "commit");
		if(!json_is_object(commit)) {
			printf("Error: Commit is not a json object\n");
			json_decref(root);
			return 1;
		}
		// COMMIT MESSAGE
		message = json_object_get(commit, "message");
		if(!json_is_string(message)) {
			printf("Error: Commit message is not a string\n");
			json_decref(root);
			return 1;
		}
		// PRINT
		message_text = json_string_value(message);
		printf("%.8s %.*s\n", json_string_value(sha), newline_offset(message_text), message_text);
	}

	// Free json object
	json_decref(root);
	return 0;

}
