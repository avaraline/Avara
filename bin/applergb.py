import numpy as np


def applergb_to_srgb(v):
    c = np.array(v)
    convmat = np.array([[1.06870538834699, 0.024110476735, 0.00173499822713],
                        [-0.07859532843279, 0.96007030899244, 0.02974755969275],
                        [0.00988984558395, 0.01581936633364, 0.96851741859153]])
    linear = np.maximum(convmat.dot(c ** 1.8), np.zeros_like(np.arange(3)))
    linear_section = np.minimum(12.92 * linear, np.array([0.040449936, 0.040449936, 0.040449936]))
    exp_section = 1.055 * (linear ** (1.0 / 2.4)) - 0.055
    res = (255.0 * np.minimum(np.ones_like(np.arange(3)), np.maximum(linear_section, exp_section))).tolist()
    return [int(x) for x in res]