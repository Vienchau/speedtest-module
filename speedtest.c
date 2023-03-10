#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include <sys/stat.h>
#include "curl/curl.h"

#ifdef CROSS
#define TEST_COUNTER 10
#else
#define TEST_COUNTER 10
#endif

#define TEST_SIZE 20000000
#define URL_CURL_DOWNLOAD "http://speedtest.fpt.vn.prod.hosts.ooklaserver.net:8080/download?size="
#define URL_CURL_UPLOAD "http://speedtest.fpt.vn.prod.hosts.ooklaserver.net:8080/speedtest/upload"

// #define URL_CURL_DOWNLOAD_SSL "speedtest.fpt.vn:8080/download?size="
// #define URL_CURL_UPLOAD_SSL "speedtest.fpt.vn:8080/speedtest/upload"

#define URL_MAX_SIZE 1024
#define STR_ARR_MAX_SIZE 30
#define FILE_NAME "file"

struct speed_t
{
    curl_off_t speed;
    curl_off_t total_time;
    char url[URL_MAX_SIZE];
};

void create_url_download(char *url)
{
    char str[STR_ARR_MAX_SIZE];
    sprintf(str, "%d", TEST_SIZE);
    strcat(url, str);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static int calc_past_time(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) / 1000;
}

static void *do_download(void *ptr)
{
    struct speed_t *speed = (struct speed_t *)ptr;
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char outfilename[12] = FILE_NAME;
    curl = curl_easy_init();
    int time;
    if (curl)
    {
        fp = fopen(outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, speed->url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "speedtest");
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        printf("result: %d\n", res);
    }

    curl_off_t transfer_time;
    if (res == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &(speed->speed));
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &(speed->total_time));
        curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &transfer_time);
        fprintf(stderr, "Download speed: %0.2f Mb/s during %lu.%06lu seconds",
                (double)(speed->speed / (double)1024 / (double)1024),
                (unsigned long)(speed->total_time / 1000000),
                (unsigned long)(speed->total_time % 1000000));
        fprintf(stderr, ", Transfer time: %lu.%06lu seconds\n", (unsigned long)(transfer_time / 1000000),
                (unsigned long)(transfer_time % 1000000));
    }
    curl_easy_cleanup(curl);
    fclose(fp);
}

double speedtest_download(char *url)
{
    struct timeval before_time, after_time;

    struct speed_t speed_arr[TEST_COUNTER];
    pthread_t threads[TEST_COUNTER];
    int rc;
    pthread_attr_t attr;
    void *status;

    double sum;
    double time;
    int i;

    for (i = 0; i < TEST_COUNTER; i++)
    {
        strcpy(speed_arr[i].url, url);
    }

    gettimeofday(&before_time, NULL);
    for (i = 0; i < TEST_COUNTER; i++)
    {
        rc = pthread_create(&threads[i], NULL, do_download, (void *)&speed_arr[i]);
        if (rc)
        {
            printf("cant create thread!\n");
        }
    }
    for (i = 0; i < TEST_COUNTER; i++)
    {
        rc = pthread_join(threads[i], &status);
        if (rc)
        {
            printf("cant join thread!\n");
        }
    }
    gettimeofday(&after_time, NULL);
    double time_hand = (double)(calc_past_time(&before_time, &after_time) / (double)1000);

    time = time / (double)TEST_COUNTER;
    printf("Total time: %f seconds\n", time_hand);
    double final_speed = (double)(((TEST_SIZE / 1000000) * (double)TEST_COUNTER) / (double)time_hand) * 8;
    return final_speed;
}

static void *do_upload(void *ptr)
{
    struct speed_t *speed = (struct speed_t *)ptr;
    CURL *curl;
    CURLcode res;
    FILE *fd;
    struct stat file_info;

    fd = fopen(FILE_NAME, "r");
    if (!fd)
    {
        printf("file open fail\n");
        return NULL;
    }

    if (fstat(fileno(fd), &file_info) != 0)
    {
        printf("file open empty\n");
        return NULL;
    }
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, URL_CURL_UPLOAD);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "speedtest upload");
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &(speed->speed));
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &(speed->total_time));
        fprintf(stderr, "Upload speed: %0.2f Mb/s during %lu.%06lu seconds\n",
                (double)(speed->speed / (double)1024 / (double)1024),
                (unsigned long)(speed->total_time / 1000000),
                (unsigned long)(speed->total_time % 1000000));
    }
    curl_easy_cleanup(curl);
}

double speedtest_upload()
{
    int i;
    struct timeval before_time, after_time;
    struct speed_t speed_arr[TEST_COUNTER];
    pthread_t threads[TEST_COUNTER];
    int rc;
    pthread_attr_t attr;
    void *status;

    double sum;
    double time;
    gettimeofday(&before_time, NULL);
    for (i = 0; i < TEST_COUNTER; i++)
    {
        rc = pthread_create(&threads[i], NULL, do_upload, (void *)&speed_arr[i]);
        if (rc)
        {
            printf("cant create thread!\n");
        }
    }
    for (i = 0; i < TEST_COUNTER; i++)
    {
        rc = pthread_join(threads[i], &status);
        if (rc)
        {
            printf("cant join thread!\n");
        }
    }
    gettimeofday(&after_time, NULL);
    double time_hand = (double)(calc_past_time(&before_time, &after_time) / (double)1000);

    time = time / (double)TEST_COUNTER;
    printf("Total time: %f seconds\n", time_hand);
    double final_speed = (double)(((TEST_SIZE / 1000000) * (double)TEST_COUNTER) / (double)time_hand) * 8;

    if (remove("file") == 0)
    {
        printf("temp file removed\n");
    }
    else
    {
        printf("temp file fail to removed\n");
    }
    return final_speed;
}

int main()
{
    char url[URL_MAX_SIZE] = URL_CURL_DOWNLOAD;
    create_url_download(url);
    printf("URL: %s\n", url);
    printf("Number of threads: %d\n", TEST_COUNTER);
    double download_speed = speedtest_download(url);

    printf("Average download speed : %0.2f Mb/s\n", download_speed);
    double upload_speed = speedtest_upload();
    printf("Average upload speed : %0.2f Mb/s\n", upload_speed);
    return 0;
}