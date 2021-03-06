import sys
import time
import requests
import fake_useragent

def FileDownload(url, downloadPath):
    with open(downloadPath, "wb") as file:
        userAgent = fake_useragent.UserAgent()
        headers   = {"User-Agent": userAgent.chrome}
        response  = requests.get(url, headers=headers, stream=True)
        total     = response.headers.get("content-length")

        if total is None:
            file.write(response.content)
            return

        downloaded = 0
        total      = int(total)
        startTime  = time.time()
        for data in response.iter_content(chunk_size=max(int(total/1000), 1024 * 1024)):
            downloaded += len(data)
            file.write(data)
            done = int(50*downloaded/total)
            percentage = (downloaded/total) * 100
            elapsedTime = time.time() - startTime
            avgKBPerSecond = (downloaded / 1024) / elapsedTime
            avgSpeedString = '{:.2f} KB/s'.format(avgKBPerSecond)
            if (avgKBPerSecond > 1024):
                avgMBPerSecond = avgKBPerSecond / 1024
                avgSpeedString = '{:.2f} MB/s'.format(avgMBPerSecond)
            sys.stdout.write('\r[{}{}] {:.2f}% ({})   '.format('█' * done, '.' * (50-done), percentage, avgSpeedString))
            sys.stdout.flush()
        sys.stdout.write('\n')


def YesNoQuestion():
    while True:
        reply = str(input('[Y/N]: ')).lower().strip()
        if reply[:1] == 'y':
            return True
        if reply[:1] == 'n':
            return False

        
