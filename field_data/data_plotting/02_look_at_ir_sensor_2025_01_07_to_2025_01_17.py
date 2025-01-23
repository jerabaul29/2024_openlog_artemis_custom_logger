# %%

exit()

# %%

# ipython3 --pdb
ipython3

# %%

from loguru import logger

from pathlib import Path
import datetime

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import scipy as sp

# %%

def interpolate_variable_to_newtimes(times: np.ndarray[np.datetime64],
                                     variable_on_times: np.ndarray,
                                     newtimes: np.ndarray[np.datetime64],
                                     timegap_max: np.datetime64 = np.timedelta64(3, "h"),
                                     max_order: int = 1) -> np.ndarray:
    """Interpolate a time dependent variable to a new set of times. This is
    done in such a way that, for each new interpolation time, only data within
    [newtime-timegap_max; newtime+timegap_map], are considered. This allows
    to perform robust interpolation even if there are holes in the initial
    timeseries. At each new time, the function returns either NaN if there
    are no samples within timegap_max, or the nearest value if there is only
    1 neighbor (order 0) within timegap_max, or the linear interpolated value
    (order 1) if there are 2 neighbors or more within timegap_max with 1 on
    each side (as long as this is allowed by the max_order argument).

    TODO: allow also higher order estimates, IF there are enough points around

    The main point is to enable reasonably good interpolation that is robust.
    
    Arguments
        - times: the times on which the variable is sampled; need to
            be sorted except for the NaT values
        - variable_on_times: the value of the variable on times; needs
            to have the same length as times
        - newtimes: the new times on which to interpolate the variable
        - timegap_map: the maximum time gap between an element of newtimes
            and an elemet of times to use the element of times in the
            interpolation.
        - max_order: the maximum interpolation order. 0: use nearest data;
            1: use linear interpolation.
            
    Output
        - newtimes_interp: the interpolated variable."""

    assert len(times) == len(variable_on_times)

    valid_times = np.logical_and(
        np.isfinite(times),
        np.isfinite(variable_on_times),
    )
    variable_on_times = variable_on_times[valid_times]
    times = times[valid_times]

    assert np.all(times[:-1] <= times[1:])

    valid_newtimes = np.isfinite(newtimes)
    full_newtimes = newtimes
    newtimes = newtimes[valid_newtimes]

    there_is_a_time_close_before_newtime = np.full((len(newtimes),), False)

    for crrt_ind, crrt_newtime in enumerate(newtimes):
        difference = crrt_newtime - times
        there_is_a_time_close_before_newtime[crrt_ind] = np.any(
            np.logical_and(
                difference > np.timedelta64(0, "ns"),
                difference < timegap_max
            )
        )

    there_is_a_time_close_after_newtime = np.full((len(newtimes),), False)

    for crrt_ind, crrt_newtime in enumerate(newtimes):
        difference = times - crrt_newtime
        there_is_a_time_close_after_newtime[crrt_ind] = np.any(
            np.logical_and(
                difference > np.timedelta64(0, "ns"),
                difference < timegap_max
            )
        )

    # interpolation only takes real coordinates
    times_ns_to_s = times.astype('datetime64[ns]').astype(np.float64) / 1e9
    newtimes_ns_to_s = newtimes.astype('datetime64[ns]').astype(np.float64) / 1e9

    newtimes_interp_nearest = sp.interpolate.interp1d(times_ns_to_s, variable_on_times, kind="nearest", bounds_error=False, fill_value="extrapolate")(newtimes_ns_to_s)

    if max_order > 0:
        newtimes_interp_linear = sp.interpolate.interp1d(times_ns_to_s, variable_on_times, kind="linear", bounds_error=False, fill_value="extrapolate")(newtimes_ns_to_s)

    # array of interpolated as linearly interpolated
    newtimes_interp = np.full(np.shape(newtimes), np.nan)

    closest_valid = np.logical_or(there_is_a_time_close_before_newtime, there_is_a_time_close_after_newtime)
    newtimes_interp[closest_valid] = newtimes_interp_nearest[closest_valid]

    if max_order > 0:
        linear_valid = np.logical_and(there_is_a_time_close_before_newtime, there_is_a_time_close_after_newtime)
        newtimes_interp[linear_valid] = newtimes_interp_linear[linear_valid]

    full_newtimes_interp = np.full(np.shape(full_newtimes), np.nan)
    full_newtimes_interp[valid_newtimes] = newtimes_interp

    return full_newtimes_interp


# %%

path_ir_proto = Path.cwd() / "../prototype_sensors/2025_01_17_data_ir_mm/IR-OMBArduino_10sec_07012025-070110.csv"
path_ir_ref = Path.cwd() / "../official_reference_sensors/2025_01_17_data_ir_mm/CR1000-koffert_IR120Data_5s_Januar2025_Blindern.dat"

# %%

pd_ir_proto = pd.read_csv(path_ir_proto)
pd_ir_proto["parsed_datetime"] = [datetime.datetime.strptime(x[2:-1], "%Y/%m/%d %H:%M:%S") - datetime.timedelta(minutes=62) for x in pd_ir_proto["datetime"]]

# %%

field_proto_ir = "object_temp"
field_proto_air = "ambient_temp"
field_steinarbox_air = "PT100_temp_Avg"
field_steinarbox_ir = "IR_sensorTempTCE098"

# %%

pd_ir_steinarbox = pd.read_csv(path_ir_ref, parse_dates=[0])

# %%

plt.figure()

plt.plot(pd_ir_proto["parsed_datetime"], pd_ir_proto[field_proto_ir], label="proto ir")
plt.plot(pd_ir_proto["parsed_datetime"], pd_ir_proto[field_proto_air], label="proto air")

plt.plot(pd_ir_steinarbox["TIMESTAMP"], pd_ir_steinarbox[field_steinarbox_air], label="Steinarbox air")
plt.plot(pd_ir_steinarbox["TIMESTAMP"], pd_ir_steinarbox[field_steinarbox_ir], label="Steinarbox ir")
# plt.plot(pd_ir_ref["TIMESTAMP"], pd_ir_ref["IR_sensor_T4"], label="Steinarbox IR 2")

plt.ylabel("degrees C")
plt.legend()

plt.show()

# %%

# interpolate both to the same time scale
newtimes = np.arange(np.datetime64("2025-01-07T13:00:00"), np.datetime64("2025-01-10T07:00:00"), np.timedelta64(10, "s"))

pd_newtimes_proto_ir = interpolate_variable_to_newtimes(pd_ir_proto["parsed_datetime"].to_numpy(), pd_ir_proto[field_proto_ir].to_numpy(), newtimes)
pd_newtimes_proto_air = interpolate_variable_to_newtimes(pd_ir_proto["parsed_datetime"].to_numpy(), pd_ir_proto[field_proto_air].to_numpy(), newtimes)
#
pd_newtimes_steinarbox_ir = interpolate_variable_to_newtimes(pd_ir_steinarbox["TIMESTAMP"].to_numpy(), pd_ir_steinarbox[field_steinarbox_ir].to_numpy(), newtimes)
pd_newtimes_steinarbox_air = interpolate_variable_to_newtimes(pd_ir_steinarbox["TIMESTAMP"].to_numpy(), pd_ir_steinarbox[field_steinarbox_air].to_numpy(), newtimes)

# %%

pd_newtimes = pd.DataFrame(
    data = \
    {
        "datetime": newtimes,
        "proto_ir": pd_newtimes_proto_ir,
        "proto_air": pd_newtimes_proto_air,
        "steinarbox_ir": pd_newtimes_steinarbox_ir,
        "steinarbox_air": pd_newtimes_steinarbox_air,
    },
    # index = newtimes,
)

# %%

plt.figure()

for crrt_field in ["proto_ir", "proto_air", "steinarbox_ir", "steinarbox_air"]:
    plt.plot(pd_newtimes["datetime"], pd_newtimes[crrt_field], label=crrt_field)

plt.ylabel("temp (C)")
plt.legend()
plt.show()

# %%

plt.figure()

plt.hist(pd_newtimes["proto_ir"] - pd_newtimes["steinarbox_ir"])

plt.show()

# %%

plt.figure()

plt.scatter(pd_newtimes["proto_ir"], pd_newtimes["steinarbox_ir"])

plt.show()

# %%


def scatter_with_trend(x, y):
    data , x_e, y_e = np.histogram2d( x, y, bins=20, density=True )
    z = sp.interpolate.interpn( ( 0.5*(x_e[1:] + x_e[:-1]) , 0.5*(y_e[1:]+y_e[:-1]) ) , data , np.vstack([x,y]).T , method = "splinef2d", bounds_error = False)
    z[np.where(np.isnan(z))] = 0.0

    # Sort the points by density, so that the densest points are plotted last
    if True:
        idx = z.argsort()
        x, y, z = x[idx], y[idx], z[idx]

    plt.scatter(x, y, c=z)

    # the trend on it: linear polyfit
    # order = 1
    # coeffs = np.polyfit(x, y, order)
    # linear_fit = np.poly1d(coeffs)
    # easier with sp.stats
    slope, intercept, rsquared, p_value, std_err = sp.stats.linregress(x, y)

    def linear_fit(x):
        return slope * x + intercept
    
    
    minx = np.min(x)
    maxx = np.max(x)

    rsquared = float(rsquared)

    plt.plot([minx, maxx], [linear_fit(minx), linear_fit(maxx)], color="red", linewidth=3, label=f"R-squared: {rsquared:.2f} | slope: {slope:.2f}")

# %%

plt.figure()

scatter_with_trend(pd_newtimes["proto_ir"] - pd_newtimes["proto_air"], pd_newtimes["proto_ir"] - pd_newtimes["steinarbox_ir"])
plt.ylabel("IR: proto-steinar")
plt.xlabel("proto: IR-ambient")

plt.legend()

plt.show()

# %%


def plot_hist_mismatch(predictor, target, label_base="", bins=30, color=None):
    mismatch = target - predictor
    mismatch_mean = np.mean(mismatch)
    mismatch_std = np.std(mismatch)
    mismatch_skew = sp.stats.skew(mismatch)
    mean_linewidth = 2
    if color is None:
        plt.hist(mismatch, bins=bins, label=label_base + f" err: mean: {mismatch_mean:.2f}; std: {mismatch_std:.2f}; skew: {mismatch_skew:.2f}", histtype=u'step')
        plt.axvline(mismatch_mean, linewidth=mean_linewidth)
    else:
        plt.hist(mismatch, bins=bins, label=label_base + f" err: mean: {mismatch_mean:.2f}; std: {mismatch_std:.2f}; skew: {mismatch_skew:.2f}", histtype=u'step', color=color)
        plt.axvline(mismatch_mean, color=color, linewidth=mean_linewidth)

    plt.xlabel("mismatch")
    plt.ylabel("count")


# %%

# how does it look with and without the simple calibration?
# method 1: do 2 calibration one after the other
# 1) linear calibration proto ir vs. steinar ir
plt.figure()
scatter_with_trend(pd_newtimes["proto_ir"], pd_newtimes["steinarbox_ir"])
plt.xlabel("proto_ir")
plt.ylabel("steinar_ir")
plt.show()
#
slope_1, intercept_1, rsq_1, pvl_1, std_1 = sp.stats.linregress(pd_newtimes["proto_ir"], pd_newtimes["steinarbox_ir"])
pd_newtimes["proto_ir_cal1"] = intercept_1 + slope_1 * pd_newtimes["proto_ir"]
#
print(f"{slope_1 = }")
print(f"{intercept_1 = }")
# 2) linear calibration to effect of proto_ir - proto_air
plt.figure()
scatter_with_trend(pd_newtimes["proto_ir"]-pd_newtimes["proto_air"], pd_newtimes["steinarbox_ir"]-pd_newtimes["proto_ir_cal1"])
plt.xlabel("proto_ir")
plt.ylabel("steinar_ir")
plt.show()
#
slope_2, intercept_2, rsq_2, pvl_2, std_2 = sp.stats.linregress(pd_newtimes["proto_ir"]-pd_newtimes["proto_air"], pd_newtimes["steinarbox_ir"]-pd_newtimes["proto_ir_cal1"])
pd_newtimes["proto_ir_cal1_cal2"] = pd_newtimes["proto_ir_cal1"] + intercept_2 + slope_2 * (pd_newtimes["proto_ir"]-pd_newtimes["proto_air"])
#
print(f"{slope_2 = }")
print(f"{intercept_2 = }")

# TODO: method 2: do a bilinear calibration at once; linear regression? other? may be more sensitive to spurious effects, start from a given point?

plt.figure()

plt.hist(pd_newtimes["proto_ir"] - pd_newtimes["steinarbox_ir"], bins=20, label="uncalibrated")
plt.hist(pd_newtimes["proto_ir_cal1"] - pd_newtimes["steinarbox_ir"], bins=20, label="cal_1")
plt.hist(pd_newtimes["proto_ir_cal1_cal2"] - pd_newtimes["steinarbox_ir"], bins=20, label="cal_2")

plt.xlabel("error (degrees C) proto vs. steinar")
plt.legend()

plt.show()

plt.figure()
plot_hist_mismatch(pd_newtimes["proto_ir"], pd_newtimes["steinarbox_ir"], bins=30, label_base="uncalibrated", color="r")
plot_hist_mismatch(pd_newtimes["proto_ir_cal1"], pd_newtimes["steinarbox_ir"], bins=30, label_base="cal_1", color="orange")
plot_hist_mismatch(pd_newtimes["proto_ir_cal1_cal2"], pd_newtimes["steinarbox_ir"], bins=30, label_base="cal_2", color="k")
plt.legend(loc="lower left",  framealpha=0.99)
plt.show()

# %%
