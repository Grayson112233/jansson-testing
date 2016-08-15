import sys
import json
import requests

def main():
    url = "https://api.github.com/repos/" + sys.argv[1] + "/" + sys.argv[2] + "/commits"
    request = requests.get(url)
    commits = json.loads(request.text)
    for commit in commits:
        print(commit['sha'][:8] + " " + commit['commit']['message'].split('\n')[0])

main()

