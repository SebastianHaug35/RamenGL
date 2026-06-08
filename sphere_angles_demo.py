import math
import tkinter as tk
from tkinter import ttk


def sphere_point(radius: float, theta_rad: float, phi_rad: float) -> tuple[float, float, float]:
    sin_phi = math.sin(phi_rad)
    x = radius * math.cos(theta_rad) * sin_phi
    y = radius * math.sin(theta_rad) * sin_phi
    z = radius * math.cos(phi_rad)
    return x, y, z


class SphereAnglesDemo:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Sphere Angles Demo")
        self.root.geometry("1280x900")

        self.theta_deg = tk.DoubleVar(value=45.0)
        self.phi_deg = tk.DoubleVar(value=60.0)
        self.radius = tk.DoubleVar(value=1.0)

        self.coord_text = tk.StringVar()
        self.angle_text = tk.StringVar()

        self._build_ui()
        self._update_view()

    def _build_ui(self) -> None:
        frame = ttk.Frame(self.root, padding=16)
        frame.pack(fill=tk.BOTH, expand=True)

        controls = ttk.Frame(frame)
        controls.pack(fill=tk.X)

        ttk.Label(controls, text="theta (rotation around z-axis)").grid(row=0, column=0, sticky=tk.W)
        theta_scale = ttk.Scale(
            controls,
            from_=0.0,
            to=360.0,
            variable=self.theta_deg,
            command=lambda _value: self._update_view(),
        )
        theta_scale.grid(row=0, column=1, sticky=tk.EW, padx=8)
        ttk.Label(controls, textvariable=self.theta_deg, width=8).grid(row=0, column=2, sticky=tk.E)

        ttk.Label(controls, text="phi (polar angle from +z downward)").grid(row=1, column=0, sticky=tk.W)
        phi_scale = ttk.Scale(
            controls,
            from_=0.0,
            to=180.0,
            variable=self.phi_deg,
            command=lambda _value: self._update_view(),
        )
        phi_scale.grid(row=1, column=1, sticky=tk.EW, padx=8)
        ttk.Label(controls, textvariable=self.phi_deg, width=8).grid(row=1, column=2, sticky=tk.E)

        ttk.Label(controls, text="radius").grid(row=2, column=0, sticky=tk.W)
        radius_scale = ttk.Scale(
            controls,
            from_=0.2,
            to=2.0,
            variable=self.radius,
            command=lambda _value: self._update_view(),
        )
        radius_scale.grid(row=2, column=1, sticky=tk.EW, padx=8)
        ttk.Label(controls, textvariable=self.radius, width=8).grid(row=2, column=2, sticky=tk.E)

        controls.columnconfigure(1, weight=1)

        info = ttk.Frame(frame, padding=(0, 12, 0, 12))
        info.pack(fill=tk.X)
        ttk.Label(info, textvariable=self.angle_text).pack(anchor=tk.W)
        ttk.Label(info, textvariable=self.coord_text).pack(anchor=tk.W)

        explanation = (
            "OpenGL-like camera view: x points right, y points up, and z is depth.  "
            "The sphere formula itself still uses theta around z and phi from +z downward."
        )
        ttk.Label(info, text=explanation, wraplength=900, foreground="#444444").pack(anchor=tk.W, pady=(6, 0))

        self.canvas = tk.Canvas(frame, width=1180, height=700, bg="white", highlightthickness=1)
        self.canvas.pack(fill=tk.BOTH, expand=True)

    def _rotate_y(self, point: tuple[float, float, float], angle_rad: float) -> tuple[float, float, float]:
        x, y, z = point
        cos_a = math.cos(angle_rad)
        sin_a = math.sin(angle_rad)
        return x * cos_a + z * sin_a, y, -x * sin_a + z * cos_a

    def _rotate_x(self, point: tuple[float, float, float], angle_rad: float) -> tuple[float, float, float]:
        x, y, z = point
        cos_a = math.cos(angle_rad)
        sin_a = math.sin(angle_rad)
        return x, y * cos_a - z * sin_a, y * sin_a + z * cos_a

    def _project(self, x: float, y: float, z: float, scale: float, center_x: float, center_y: float) -> tuple[float, float]:
        view = self._rotate_y((x, y, z), math.radians(-35.0))
        view = self._rotate_x(view, math.radians(22.0))

        camera_distance = 5.0
        depth = camera_distance - view[2]
        perspective = scale / max(depth, 0.25)

        screen_x = center_x + view[0] * perspective
        screen_y = center_y - view[1] * perspective
        return screen_x, screen_y

    def _draw_axis(self, center_x: float, center_y: float, scale: float, axis: tuple[float, float, float], color: str, label: str) -> None:
        x0, y0 = self._project(0.0, 0.0, 0.0, scale, center_x, center_y)
        x1, y1 = self._project(axis[0], axis[1], axis[2], scale, center_x, center_y)
        self.canvas.create_line(x0, y0, x1, y1, width=2, fill=color, arrow=tk.LAST)
        self.canvas.create_text(x1 + 12, y1, text=label, fill=color, font=("Segoe UI", 10, "bold"))

    def _update_view(self) -> None:
        theta_rad = math.radians(self.theta_deg.get())
        phi_rad = math.radians(self.phi_deg.get())
        radius = self.radius.get()
        point = sphere_point(radius, theta_rad, phi_rad)

        self.angle_text.set(
            f"theta = {self.theta_deg.get():6.2f} deg, phi = {self.phi_deg.get():6.2f} deg, radius = {radius:4.2f}"
        )
        self.coord_text.set(
            f"point = (x={point[0]: .4f}, y={point[1]: .4f}, z={point[2]: .4f})"
        )

        self.canvas.delete("all")
        width = max(self.canvas.winfo_width(), 1180)
        height = max(self.canvas.winfo_height(), 700)
        center_x = width * 0.5
        center_y = height * 0.58
        scale = min(width, height) * 0.34

        self.canvas.create_text(
            center_x,
            24,
            text="Spherical coordinates in an OpenGL-like y-up camera view",
            font=("Segoe UI", 14, "bold"),
        )

        sphere_radius_px = radius * scale
        self.canvas.create_oval(
            center_x - sphere_radius_px,
            center_y - sphere_radius_px,
            center_x + sphere_radius_px,
            center_y + sphere_radius_px,
            outline="#c8c8c8",
            width=2,
        )

        self._draw_axis(center_x, center_y, scale * radius, (1.25, 0.0, 0.0), "#d12c2c", "x")
        self._draw_axis(center_x, center_y, scale * radius, (0.0, 1.25, 0.0), "#1d8a34", "y")
        self._draw_axis(center_x, center_y, scale * radius, (0.0, 0.0, 1.25), "#1d4fd1", "z")

        origin = self._project(0.0, 0.0, 0.0, scale, center_x, center_y)
        point_2d = self._project(point[0], point[1], point[2], scale, center_x, center_y)
        xy_projection = self._project(point[0], point[1], 0.0, scale, center_x, center_y)
        z_projection = self._project(0.0, 0.0, point[2], scale, center_x, center_y)

        self.canvas.create_line(origin[0], origin[1], point_2d[0], point_2d[1], fill="#202020", width=3)
        self.canvas.create_line(origin[0], origin[1], xy_projection[0], xy_projection[1], fill="#777777", dash=(5, 3))
        self.canvas.create_line(xy_projection[0], xy_projection[1], point_2d[0], point_2d[1], fill="#999999", dash=(5, 3))
        self.canvas.create_line(z_projection[0], z_projection[1], point_2d[0], point_2d[1], fill="#bbbbbb", dash=(2, 3))

        self.canvas.create_oval(point_2d[0] - 6, point_2d[1] - 6, point_2d[0] + 6, point_2d[1] + 6, fill="#ff8c00", outline="")
        self.canvas.create_text(point_2d[0] + 24, point_2d[1] - 10, text="P(theta, phi)", font=("Segoe UI", 10, "bold"))

        self.canvas.create_text(
            18,
            height - 90,
            anchor=tk.W,
            justify=tk.LEFT,
            text=(
                "Reading the sketch:\n"
                "- black line: radius from origin to the selected point\n"
                "- dashed gray line in the base: projection into the xy-plane where theta acts\n"
                "- y is drawn upward like a typical OpenGL scene, while z is depth in the camera view"
            ),
            font=("Segoe UI", 10),
        )


def main() -> None:
    root = tk.Tk()
    app = SphereAnglesDemo(root)
    root.minsize(1080, 760)
    root.mainloop()


if __name__ == "__main__":
    main()