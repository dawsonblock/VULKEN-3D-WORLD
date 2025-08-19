



from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

 

from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

 

        main
        main
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
        main
"""Minimal capsule representation for tests."""
        main


@dataclass
class Capsule:
    """Vertical capsule defined by its center, half height and radius.


    The capsule is aligned along the Y axis.  ``center`` represents the middle
    of the cylindrical part, ``half_height`` is the half length of this
    cylinder and ``radius`` is the radius of the spherical caps and the
    cylinder.
    """



import numpy as np
        main
        main
from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

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
        main
from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


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

import numpy as np
from numpy.typing import NDArray


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

  
  
         main
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
        """Center of the top spherical cap."""

        """Center of the top spherical 
 



         main
         main
         main
         main
         main
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

         main
        return self.center + np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Center of the bottom spherical cap."""
        return self.center - np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )


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
        main
        main
        main
        main
