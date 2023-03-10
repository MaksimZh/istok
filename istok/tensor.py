from typing import Any
import numpy as np
from nptyping import NDArray

from istok.tools import Status, status

class Tensor(Status):

    __array: NDArray[Any, Any]
    __axis_names: tuple[str, ...]
    __axis_indices: dict[str, int]

    # CONSTRUCTOR
    def __init__(self, array: NDArray[Any, Any], axis_names: tuple[str, ...]) -> None:
        super().__init__()
        assert array.ndim == len(axis_names)
        self.__array = array
        self.__axis_names = axis_names
        self.__axis_indices = dict((axis_names[i], i) for i in range(len(axis_names)))


    # QUERIES

    # Get number of dimensions
    def get_ndim(self) -> int:
        return self.__array.ndim

    # Get ordered names of the axes
    def get_axis_names(self) -> tuple[str, ...]:
        return self.__axis_names
    
    # Get size of axis
    def get_size(self, axis: str) -> int:
        return self.__array.shape[self.__axis_indices[axis]]

    # Get numpy array with optionally transposed and/or additional axes
    @status("OK", "ERR")
    def get_array(self, *axes: str | tuple[str, int]) -> NDArray[Any, Any]:
        if len(axes) == 0 or tuple(axes) == self.__axis_names:
            self._set_status("get_array", "OK")
            return self.__array
        if len(axes) < len(self.__axis_names):
            self._set_status("get_array", "ERR")
            return np.array(None)
        indices = list[int]()
        slices = list[int | slice | type(np.newaxis)]()
        for ax in axes:
            if isinstance(ax, tuple) and ax[0] in self.__axis_indices:
                indices.append(self.__axis_indices[ax[0]])
                slices.append(ax[1])
                continue
            assert isinstance(ax, str)
            if ax in self.__axis_indices:
                indices.append(self.__axis_indices[ax])
                slices.append(slice(None))
                continue
            if ax.startswith("*") and ax[1:] not in self.__axis_indices:
                slices.append(np.newaxis)
                continue
            self._set_status("get_array", "ERR")
            return np.array(None)
        self._set_status("get_array", "OK")
        return self.__array.transpose(*indices)[*slices]

    # Create copy of the tensor with shallow copy of the data
    def copy(self) -> "Tensor":
        return Tensor(self.__array.copy(), self.__axis_names)
