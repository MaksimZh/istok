from typing import Any
import numpy as np
from nptyping import NDArray

from istok.tools import Status, status

class Tensor(Status):

    __array: NDArray[Any, Any]
    __dim_names: tuple[str, ...]
    __dim_indices: dict[str, int]

    def __init__(self, array: NDArray[Any, Any], dim_names: tuple[str, ...]) -> None:
        super().__init__()
        self.__array = array
        self.__dim_names = dim_names
        self.__dim_indices = dict((dim_names[i], i) for i in range(len(dim_names)))

    @status("OK", "ERR")
    def get_array(self, *dims: str) -> NDArray[Any, Any]:
        if len(dims) == 0 or tuple(dims) == self.__dim_names:
            self._set_status("get_array", "OK")
            return self.__array
        if len(dims) < len(self.__dim_names):
            self._set_status("get_array", "ERR")
            return np.array([])
        indices = list[int]()
        slices = list[slice | type(np.newaxis)]()
        for dim in dims:
            if dim in self.__dim_indices:
                indices.append(self.__dim_indices[dim])
                slices.append(slice(None))
                continue
            if dim.startswith("*") and dim[1:] not in self.__dim_indices:
                slices.append(np.newaxis)
                continue
            self._set_status("get_array", "ERR")
            return np.array([])
        self._set_status("get_array", "OK")
        return self.__array.transpose(*indices)[*slices]
