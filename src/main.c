#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h> 
#include <pthread.h>

#include "lib/typedefs.h"

#include "models.h"
#include "market.h"
#include "sticks.h"

pthread_mutex_t hdf5_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct ThreadArgs {
    string path;
    u64 interval;
    u64 post_price_time;
    f64 trigger_delta;
    StickDataArray result;
} ThreadArgs;

void print_progress_bar(unsigned long done, unsigned long total, time_t start_time) {
    int bar_width = 50;

    float progress = (float)done / total;
    int pos = (int)(bar_width * progress);

    time_t now = time(NULL);
    time_t elapsed = now - start_time;

    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;

    printf("%02d:%02d:%02d [", hours, minutes, seconds);
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("#");
        else printf("-");
    }

    f32 progress_percent = progress * 100.0;
    if (progress_percent < 100.0) {
        printf("] %lu/%lu %.2f%% \r", done, total, progress_percent);
        fflush(stdout);
    } else {
        printf("] %lu/%lu %.2f%% \n", done, total, progress_percent);
    }
}

void string_vec_free(string* v, u64 s) {
    for (u64 i=0; i < s; i++) {
        string_free(&v[i]);
    }
    free(v);
}

u64 parse_date(char* date) {
	struct tm tm = {0};
	sscanf(date, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	return mktime(&tm);
}

string* get_files(string* path, u64* count, string* file_type) {
    struct dirent* de;
    DIR* dr = opendir(path->str);
    if (dr == NULL) {
        perror("No such directory");
        return NULL;
    }

    u64 capacity = 0;
    string* files_vec = NULL;

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(".", de->d_name) == 0 || strcmp("..", de->d_name) == 0) {
            continue;
        }
        if (file_type != NULL) {
            char* find = strstr(de->d_name, file_type->str);
            if (find == NULL) continue;
        }

        files_vec = realloc(files_vec, sizeof(string)*(capacity+1));
        if (files_vec == NULL) {
            closedir(dr);
            return NULL; 
        }
        files_vec[capacity] = string_new(de->d_name);
        capacity++;
    }
    closedir(dr);

    *count = capacity;
    return files_vec;
}

StickDataArray analysis(string* path, u64 interval, u64 post_price_time, f64 trigger_delta) {
	string symbol = {0};

	string* vec_path_str = NULL;
	u64 vec_path_str_capacity = 0;
	if (SYSTEM_PLATFORM == Unix) {
		vec_path_str = string_split(path, '/', &vec_path_str_capacity);
	} else if (SYSTEM_PLATFORM == Windows) {
		vec_path_str = string_split(path, '\\', &vec_path_str_capacity);
	}
	string* vec_file_str = NULL;
	u64 vec_file_str_capacity = 0;
	vec_file_str = string_split(&vec_path_str[vec_path_str_capacity-1], '.', &vec_file_str_capacity);
	
	string* vec_symbol_str = NULL;
	u64 vec_symbol_str_capacity = 0;
	vec_symbol_str = string_split(&vec_file_str[vec_file_str_capacity-2], '_', &vec_symbol_str_capacity);

	symbol = string_new(vec_symbol_str[0].str);
    string_vec_free(vec_path_str, vec_path_str_capacity);
    string_vec_free(vec_file_str, vec_file_str_capacity);
    string_vec_free(vec_symbol_str, vec_symbol_str_capacity);

	pthread_mutex_lock(&hdf5_lock);
    TradesArray ta = read_hdf5(path->str);
    pthread_mutex_unlock(&hdf5_lock);
	SticksGenerator sticks_generator = {
		.interval = interval,
		.temp_stick = {0}
	};

	StickArray sticks = {0};
    sticks.capacity = 1000;
    sticks.length = 0;
	sticks.sticks = (Stick*)malloc(sizeof(Stick)*sticks.capacity);

    for (u64 i = 0; i < ta.capacity; i++) {
        sticks_generator_new_event(&sticks_generator, &sticks, &ta.trades[i]);
    }
    free(ta.trades);

	StickDataArray data = {0};
    data.capacity = 100;
    data.length = 0;
    data.data = (StickData*)malloc(sizeof(StickData)*data.capacity);

	u64 post_count_sticks = post_price_time / interval;
	u64 max_sticks = sticks.length;
    u64 count_5m = 300000/interval;
    u64 count_1m = 60000/interval;
    u64 count_15s = 15000/interval;

    for (u64 i=0; i < sticks.length; i++) {
        Stick* s = &sticks.sticks[i];

        if (data.length >= data.capacity) {
            data.capacity *= 2;
            data.data = realloc(data.data, sizeof(StickData) * data.capacity);
        }

        if (s->time - sticks.sticks[0].time < count_5m * interval || i >= (max_sticks-1) - post_count_sticks) {
            continue;
        }

        f64 delta = (s->high - s->low) / s->low * 100.0;
        if (delta >= trigger_delta) {
            f64 pmax_15s = s->avg;
            f64 pmin_15s = s->avg;
            f64 pmax_1m = s->avg;
            f64 pmin_1m = s->avg;
            f64 pmax_5m = s->avg;
            f64 pmin_5m = s->avg;

            for (u64 j=1; j <= i; j++) {
                Stick* sj = &sticks.sticks[i-j];

                if (s->time - sj->time <= count_15s * interval) {
                    if (sj->high > pmax_15s) pmax_15s = sj->high;
                    if (sj->low < pmin_15s) pmin_15s = sj->low;
                }
                if (s->time - sj->time <= count_1m * interval) {
                    if (sj->high > pmax_1m) pmax_1m = sj->high;
                    if (sj->low < pmin_1m) pmin_1m = sj->low;
                }
                if (s->time - sj->time <= count_5m * interval) {
                    if (sj->high > pmax_5m) pmax_5m = sj->high;
                    if (sj->low < pmin_5m) pmin_5m = sj->low;
                }
            }

            f64 delta_15s = (pmax_15s-pmin_15s)/pmin_15s*100.0;
            f64 delta_1m = (pmax_1m-pmin_1m)/pmin_1m*100.0;
            f64 delta_5m = (pmax_5m-pmin_5m)/pmin_5m*100.0;

            data.data[data.length].symbol = symbol;
            data.data[data.length].timestamp = s->time;
            if (sticks.sticks[i - 1].avg < s->avg) {
                data.data[data.length].start_price = sticks.sticks[i-1].low;
                data.data[data.length].peak_price = s->high;
            } else {
                data.data[data.length].start_price = sticks.sticks[i-1].high;
                data.data[data.length].peak_price = s->low;
            }
            for (u64 j=i; j <= i+post_count_sticks; j++) {
                if (sticks.sticks[j].time - s->time <= post_price_time)
                    data.data[data.length].post_price = sticks.sticks[j].avg;
                else break;
            }
            data.data[data.length].delta_15s = delta_15s;
            data.data[data.length].delta_1m = delta_1m;
            data.data[data.length].delta_5m = delta_5m;
            data.length++;
        }

    }
    free(sticks.sticks);

    return data;
}

void* thread_worker(void* arg) {
    ThreadArgs* ta = (ThreadArgs*)arg;
    ta->result = analysis(
        &ta->path, 
        ta->interval, 
        ta->post_price_time, 
        ta->trigger_delta
    );
    return NULL;
}

int main(int argc, char** argv) {

    u64 timestamp_from = 0;
    u64 timestamp_to = 0;
    u32 cpu = 1;
    string path_read_symbols = {0};
    u64 interval = 150;
    u64 post_price_time = 1000;
    f64 trigger_delta = 0.5;
    u8 validate_files = false; 

    for (u32 i=0; i < argc; i++) {
        if (!strcmp("--path-read-symbols", argv[i])) {
            path_read_symbols = string_new(argv[i+1]);
        } else if (!strcmp("--cpu", argv[i])) {
            sscanf(argv[i+1], "%u", &cpu);
        } else if (!strcmp("--date-from", argv[i])) {
            timestamp_from = parse_date(argv[i+1]);
        } else if (!strcmp("--date-to", argv[i])) {
            timestamp_to = parse_date(argv[i+1]);
        } else if (!strcmp("--interval", argv[i])) {
            sscanf(argv[i+1], "%lu", &interval);
        } else if (!strcmp("--post-price-time", argv[i])) {
            sscanf(argv[i+1], "%lu", &post_price_time);
        } else if (!strcmp("--trigger-delta", argv[i])) {
            sscanf(argv[i+1], "%lf", &trigger_delta);
        } else if (!strcmp("--validate-files", argv[i])) {
            validate_files = true;
        }
    }

    u64 count_files = 0;
    string file_type = string_new(".hdf5");
    string* vec_filename = get_files(&path_read_symbols, &count_files, &file_type);
    strings_sort(vec_filename , count_files);

    u64 count_paths = 0;
    string* vec_path = malloc(sizeof(string)*count_files);
    for (u64 i = 0; i < count_files; i++) {
        string* vec_symbol_date_str = NULL;
        u64 vec_symbol_date_str_capacity = 0;
        vec_symbol_date_str = string_split(&vec_filename[i], '.', &vec_symbol_date_str_capacity);
        if (vec_symbol_date_str_capacity < 2) {
            printf("Error Invalid Filename: %s\n", vec_filename[i].str);
            return 0;
        }
        
        string* vec_date_str = NULL;
        u64 vec_date_str_capacity = 0;
        vec_date_str = string_split(&vec_symbol_date_str[vec_symbol_date_str_capacity-2], '_', &vec_date_str_capacity);
        if (vec_date_str_capacity < 2) {
            printf("Error Invalid Filename (missing date): %s\n", vec_filename[i].str);
            return 0;
        }
        string* date_str = &vec_date_str[vec_symbol_date_str_capacity-1];

        u64 file_timestamp = parse_date(date_str->str);

        if ( (timestamp_from+timestamp_to == 0) || (timestamp_from <= file_timestamp && file_timestamp <= timestamp_to)) {
            vec_path[count_paths] = string_new(vec_filename[i].str);
            count_paths++;
        }
    }
    string_vec_free(vec_filename, count_files);

    for (u64 i = 0; i < count_paths; i++) {
        size_t np_capacity = vec_path[i].length + path_read_symbols.length + 2;
        char* new_path = (char*)malloc(sizeof(char) * np_capacity);

        if (SYSTEM_PLATFORM == Unix) {
            if (path_read_symbols.str[path_read_symbols.length-1] == '/') {
                snprintf(new_path, np_capacity, "%s%s", path_read_symbols.str, vec_path[i].str);
            } else { 
                snprintf(new_path, np_capacity, "%s/%s", path_read_symbols.str, vec_path[i].str);
            }
        } else if (SYSTEM_PLATFORM == Windows) {
            if (path_read_symbols.str[path_read_symbols.length-1] == '\\') {
                snprintf(new_path, np_capacity, "%s%s", path_read_symbols.str, vec_path[i].str);
            } else { 
                snprintf(new_path, np_capacity, "%s\\%s", path_read_symbols.str, vec_path[i].str);
            }
        }
        string_set(&vec_path[i], new_path);
        free(new_path);
    }

    if (validate_files) {
        time_t start_time = time(NULL);
        for (u64 i = 0; i < count_paths; i++) {
            i32 result = validate_file(vec_path[i].str);
            if (result < 0) {
                remove(vec_path[i].str);
            }
            print_progress_bar(i+1, count_paths, start_time);
        }
        return 0;
    }

    pthread_t* threads = malloc(sizeof(pthread_t) * count_paths);
    ThreadArgs* targs = malloc(sizeof(ThreadArgs) * count_paths);

    time_t start_time = time(NULL);
    for (u64 i = 0; i < count_paths; i += cpu) {
        u64 batch_end = i + cpu;
        if (batch_end > count_paths) batch_end = count_paths;
    
        for (u64 j = i; j < batch_end; j++) {
            targs[j].path = string_new(vec_path[j].str);
            targs[j].interval = interval;
            targs[j].post_price_time = post_price_time;
            targs[j].trigger_delta = trigger_delta;
    
            pthread_create(&threads[j], NULL, thread_worker, &targs[j]);
        }
    
        for (u64 j = i; j < batch_end; j++) {
            pthread_join(threads[j], NULL);
            print_progress_bar(j+1, count_paths, start_time);
        }
    }

    FILE *file = fopen("data.csv", "w");
    fprintf(file, "symbol,timestamp,start_price,peak_price,post_price,delta_15s,delta_1m,delta_5m\n");

    for (u64 i = 0; i < count_paths; i++) {
        StickDataArray* sda = &targs[i].result;
        for (u64 k = 0; k < sda->length; k++) {
            fprintf(file, "%s,%lu,%.18lf,%.18lf,%.18lf,%.2lf,%.2lf,%.2lf\n",
                sda->data[k].symbol.str,
                sda->data[k].timestamp,
                sda->data[k].start_price,
                sda->data[k].peak_price,
                sda->data[k].post_price,
                sda->data[k].delta_15s,
                sda->data[k].delta_1m,
                sda->data[k].delta_5m
            );
        }
        free(sda->data);
    }
    fclose(file);

    free(threads);
    free(targs);
    string_vec_free(vec_path, count_paths);
    
    return 0;
}
