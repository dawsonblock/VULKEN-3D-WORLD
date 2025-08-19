 

from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

 

        main
"""Minimal capsule representation for tests."""



from __future__ import annotations


from __future__ import annotations

from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by a center point, half-height, and radius."""

 



from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray
        main

        main

        main
        main
        main
        main
        main
"""Minimal capsule representation for tests."""


@dataclass
class Capsule:


import numpy as np
from dataclasses import dataclass

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

        main
        main
from __future__ import annotations



        main
        main
        main
        main
from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray



@dataclass
class Capsule:
    """Vertical capsule defined by center, half-height, and radius."""

import numpy as np
        main
from numpy.typing import NDArray
        main
        main

import numpy as np
from numpy.typing import NDArray

import numpy as np
from numpy.typing import NDArray

import numpy as np
from numpy.typing import NDArray

import numpy as np
from numpy.typing import NDArray
        main


@dataclass
class Capsule:

    """Simple vertical capsule defined by its center, half-height, and radius.


    """Vertical capsule defined by its center, half-height, and radius."""



    """Vertical capsule defined by its center, half-height, and radius."""



    """Simple vertical capsule defined by its center,
    half-height, and radius."""
    """
    Represents a simple vertical capsule defined by its center, half-height, and radius.


    """Vertical capsule defined by its center, half-height, and radius."""


    """Vertical capsule defined by its center, half-height, and radius."""

    """Vertical capsule defined by its center, half-height, and radius."""

  
  
    """Vertical capsule represented by a center point and radius.
         main
        main

    Parameters
    ----------
    center : numpy.ndarray of shape (3,)
        The center of the capsule (midpoint between the two spherical caps), in 3D space.
    half_height : float
        Half the height of the cylindrical part of the capsule (distance from center to cap center).
    radius : float
        The radius of the spherical caps and the cylinder.
    """

 


        main
        main
    """Simple vertical capsule defined by its center, half-height, and radius."""
        main
        main
        main
        main
        main

    center: NDArray[np.float32, Literal[3]]

        main
        main
        main
        main
        main
    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Center of the top spherical cap."""

 



         main
         main
         main
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

        return self.center + np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )
         main

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Center of the bottom spherical cap."""


        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)

        return self.center - np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )
        main


__all__ = ["Capsule"]

 











        main
        main
        main
        main
        main
        main
        main
        main
        main
        main
        main
        main
