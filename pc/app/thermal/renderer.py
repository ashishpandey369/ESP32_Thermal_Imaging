import numpy as np
import pyqtgraph as pg


class ThermalRenderer:
    def __init__(self, image_widget: pg.ImageView):
        self.view = image_widget
        self.view.ui.roiBtn.hide()
        self.view.ui.menuBtn.hide()
        self.view.setColorMap(pg.colormap.get('inferno'))

        # Keep the original 32x24 sensor data intact, but interpolate it only
        # for display so the thermal image appears continuous at large size.
        # This does not create extra measurement data or change temperatures.
        image_item = self.view.getImageItem()
        image_item.setOpts(interpolation='linear', autoDownsample=False)

    def update(self, frame):
        image = np.asarray(frame['pixels'], dtype=np.float32).reshape((24, 32))
        self.view.setImage(
            image,
            autoLevels=False,
            levels=(frame['minimum'], frame['maximum']),
        )
