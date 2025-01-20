# %%

exit()

# %%

# ipython3 --pdb
ipython3

# %%

from loguru import logger

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

# %%

import datetime
import pytz

utc_timezone = pytz.timezone("UTC")

# %%

path_prototype_data = Path.cwd() / "../prototype_sensors/2025_01_17_data/"
path_reference_data = Path.cwd() / "../official_reference_sensors/2025_01_17_data/"

# %%

# load reference data

# - C: CNR4
# - A: Apoge

# - T: temperature
# - Q: radiation

# - I: incoming
# - O: outgoing
# - S: shortwave
# - L: longwave
# - N: net

file_ref_temp = path_reference_data / "blindern_data_3205059_34_PT100_rs1_test_2025_01_07__to_2025_01_17.csv"
# file_ref_temp = path_reference_data / "blindern_data_0_1_PT100_rs2_stevenson_test_2025_01_07__to_2025_01_17.csv"
file_ref_solar_CQSI = path_reference_data / "blindern_data_0_15_C_QSI_test_2025_01_07__to_2025_01_17.csv"
file_ref_solar_AQSI = path_reference_data / "blindern_data_0_4_A_QSI_test_2025_01_07__to_2025_01_17.csv"

pd_ref_temp = pd.read_csv(file_ref_temp, parse_dates=[1])
pd_ref_CQSI = pd.read_csv(file_ref_solar_CQSI, parse_dates=[1])
pd_ref_AQSI = pd.read_csv(file_ref_solar_AQSI, parse_dates=[1])

# %%

plt.figure()

plt.plot(pd_ref_temp["Timestamp"], pd_ref_temp["Value"].to_numpy(), label="temp (C)")

plt.plot(pd_ref_CQSI["Timestamp"], pd_ref_CQSI["Value"].to_numpy(), label="CQSI (W/m2)")
plt.plot(pd_ref_AQSI["Timestamp"], pd_ref_AQSI["Value"].to_numpy(), label="AQSI (W/m2)")

plt.legend()
plt.show()

# %%

# parse all the sensors data

list_files_prototype = sorted(path_prototype_data.glob("*.dat"))
list_csvs = []

for crrt_file_prototype in list_files_prototype:
    # logger.info(f"process {crrt_file_prototype}")

    # check that these are the standard files we expect
    with open(crrt_file_prototype, "br") as f:
        num_lines = sum(1 for _ in f)

    assert num_lines == 41

    # we are interested in specific bits:

    # get the posix timestamp
    with open(crrt_file_prototype, "br") as f:
        for i, line in enumerate(f):
            if i == 5:
                break

    crrt_posix = int(line[30:-2])
    pd_datetime = pd.to_datetime(crrt_posix, unit='s')

    # get the CSV data
    crrt_data = pd.read_csv(crrt_file_prototype, skiprows=6, skipfooter=4, engine="python")

    crrt_data["Timestamp"] = [pd_datetime + pd.Timedelta(seconds=int(secs/5)) for secs in crrt_data["READING_NBR"].to_numpy()]

    list_csvs.append(crrt_data)

pd_proto_data = pd.concat(list_csvs, ignore_index=True)

# %%

lookup_ID_to_name = {
    2893986434666987807: "small shield (hanging) metal",
    2935393797722341677: "medium shield metal",
    2902201770801365469: "small shield (side) metal",
    2910065340557820165: "medium shield metal",
    2918313179895300117: "small shield (side) black"
}

# %%

plt.figure()

for crrt_id in lookup_ID_to_name.keys():
    pd_proto_data_crrt_sensor = pd_proto_data[pd_proto_data["THERMISTOR_ID"] == crrt_id]
    pd_proto_data_crrt_sensor_name = lookup_ID_to_name[crrt_id]

    plt.plot(pd_proto_data_crrt_sensor["Timestamp"], pd_proto_data_crrt_sensor["CELCIUS"], label=pd_proto_data_crrt_sensor_name)

plt.legend()
plt.show()

# %%

# TODO: use the different reference temperature measurements
# TODO: use several radiation information data
# TODO: show wind also, plays a role in effective cooling

# print it all together

plt.figure()

plt.plot(pd_ref_temp["Timestamp"], pd_ref_temp["Value"].to_numpy(), label="temp (C)")

plt.plot(pd_ref_CQSI["Timestamp"], pd_ref_CQSI["Value"].to_numpy(), label="CQSI (W/m2)")
plt.plot(pd_ref_AQSI["Timestamp"], pd_ref_AQSI["Value"].to_numpy(), label="AQSI (W/m2)")

for crrt_id in lookup_ID_to_name.keys():
    pd_proto_data_crrt_sensor = pd_proto_data[pd_proto_data["THERMISTOR_ID"] == crrt_id]
    pd_proto_data_crrt_sensor_name = lookup_ID_to_name[crrt_id]

    plt.plot(pd_proto_data_crrt_sensor["Timestamp"], pd_proto_data_crrt_sensor["CELCIUS"], label=pd_proto_data_crrt_sensor_name)

plt.legend()
plt.show()

# %%

# TODO: statistical analysis of the mismatch

# %%

