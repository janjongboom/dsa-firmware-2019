    // uint16_t pm10_standard, pm25_standard, pm100_standard;
    // uint16_t pm10_env, pm25_env, pm100_env;
    // uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;

    // // 12x 2 bytes

let data = process.argv[2];

data = data.split(/(..)/g).filter(f=>!!f).map(f => parseInt(f, 16));
data = data.slice(data.length - 24);

let d = {
    pm10_standard: data[0] + (data[1] << 8),
    pm25_standard: data[2] + (data[3] << 8),
    pm100_standard: data[4] + (data[5] << 8),
    pm10_env: data[6] + (data[7] << 8),
    pm25_env: data[8] + (data[9] << 8),
    pm100_env: data[10] + (data[11] << 8),
    pm10_env: data[6] + (data[7] << 8),
    pm25_env: data[8] + (data[9] << 8),
    pm100_env: data[10] + (data[11] << 8),
    particles_03um: data[12] + (data[13] << 8),
    particles_05um: data[14] + (data[15] << 8),
    particles_10um: data[16] + (data[17] << 8),
    particles_25um: data[18] + (data[19] << 8),
    particles_50um: data[20] + (data[21] << 8),
    particles_100um: data[22] + (data[23] << 8),
};

console.log(d);

// c016700DE02685E036700E604731D1E0565052F060300170703001907000B000D0007000B000D00CA0592014400060004000000