#include "market.h"
#include "lib/typedefs.h"

TradesArray read_hdf5(char* path) {
    TradesArray trades_array = {0};

    hid_t fid = H5Fopen(path, H5F_ACC_RDONLY, H5P_DEFAULT);

    hsize_t dims[1];
    H5LTget_dataset_info(fid, "id", dims, NULL, NULL);
    u64 capacity = (u64)dims[0];

    u64* ids = malloc(sizeof(u64) * capacity);
    u64* timestamps = malloc(sizeof(u64) * capacity);
    f64* prices = malloc(sizeof(f64) * capacity);
    f64* quantitys = malloc(sizeof(f64) * capacity);
    i32* is_markets = malloc(sizeof(int32_t) * capacity);

    H5LTread_dataset(fid, "id", H5T_NATIVE_UINT64, ids);
    H5LTread_dataset(fid, "timestamp", H5T_NATIVE_UINT64, timestamps);
    H5LTread_dataset(fid, "price", H5T_NATIVE_DOUBLE, prices);
    H5LTread_dataset(fid, "quantity", H5T_NATIVE_DOUBLE, quantitys);
    H5LTread_dataset(fid, "is_market", H5T_NATIVE_INT32, is_markets);
    
    H5Fclose(fid);
    H5garbage_collect();
    H5close();

    trades_array.trades = (Trade*)malloc(sizeof(Trade) * capacity);
    trades_array.capacity = capacity;
    trades_array.length = capacity;

    for (u64 i = 0; i < capacity; i++) {
        trades_array.trades[i].id = ids[i];
        trades_array.trades[i].timestamp = timestamps[i];
        trades_array.trades[i].price = prices[i];
        trades_array.trades[i].quantity = quantitys[i];
        trades_array.trades[i].is_market = is_markets[i];
    }

    free(ids);
    free(timestamps);
    free(prices);
    free(quantitys);
    free(is_markets);

    return trades_array;
}

i32 validate_file(char* path) {
    H5Eset_auto(H5E_DEFAULT, NULL, NULL);

	hid_t fid = H5Fopen(path, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fid < 0) {
		printf("Error Open File: %s \n", path);
		return -1; 
	}

	const char* datasets[] = {"id", "timestamp", "price", "quantity", "is_market"};
	const H5T_class_t expected_types[] = {H5T_INTEGER, H5T_INTEGER, H5T_FLOAT, H5T_FLOAT, H5T_ENUM};
	size_t count = sizeof(datasets) / sizeof(datasets[0]);

	for (size_t i = 0; i < count; i++) {
        if (H5Lexists(fid, datasets[i], H5P_DEFAULT) <= 0) {
            printf("Error Missing Dataset: %s, path: %s \n", datasets[i], path);
            H5Fclose(fid);
            return -1;
        }

        hsize_t dims[1];
        H5T_class_t class_id;
        size_t type_size;
        if (H5LTget_dataset_info(fid, datasets[i], dims, &class_id, &type_size) < 0) {
            printf("Error Corrupted Dataset: %s, path: %s \n", datasets[i], path);
            H5Fclose(fid);
            return -1;
        }

        if (class_id != expected_types[i]) {
            printf("Error Invalid Type: %s, path: %s \n", datasets[i], path);
            H5Fclose(fid);
            return -1;
        }

        if (dims[0] == 0) {
            printf("Error Empty Dataset: %s, path:%s \n", datasets[i], path);
            H5Fclose(fid);
            return -1;
        }
    }
	H5Fclose(fid);
	return 0;
}
