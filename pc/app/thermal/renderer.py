import numpy as np
import pyqtgraph as pg
from PySide6.QtCore import Qt


class ThermalRenderer:
    def __init__(self, image_widget: pg.ImageView):
        self.view = image_widget
        self.view.ui.roiBtn.hide()
        self.view.ui.menuBtn.hide()
        self.view.setColorMap(pg.colormap.get('inferno'))
        self.view.setPredefinedGradient('inferno')

    def update(self, frame):
        image = np.asarray(frame['pixels'], dtype=np.float32)
        self.view.setImage(
            image,
            autoLevels=False,
            levels=(frame['minimum'], frame['maximum']),
        )
