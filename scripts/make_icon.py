"""Generate app/icon.png for the Onion Apps list.

Run once on the host (needs Pillow):  uv run --with pillow scripts/make_icon.py
The generated PNG is committed, so the build itself does not need Python.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
SIZE = 80
SCALE = 4  # draw large, then downsample for smooth edges

big = SIZE * SCALE
img = Image.new("RGBA", (big, big), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
d.rounded_rectangle((0, 0, big - 1, big - 1), radius=18 * SCALE, fill=(22, 24, 34, 255))

font = ImageFont.truetype(str(ROOT / "res/fonts/Exo2-Bold.ttf"), 22 * SCALE)
half = 34 * SCALE  # square side


def panel(upside_down: bool) -> Image.Image:
    p = Image.new("RGBA", (half, half), (0, 0, 0, 0))
    pd = ImageDraw.Draw(p)
    pd.rounded_rectangle((0, 0, half - 1, half - 1), radius=5 * SCALE,
                         fill=(32, 35, 48, 255), outline=(255, 196, 60, 255), width=2 * SCALE)
    pd.text((half // 2, half // 2), "20", font=font, fill=(240, 240, 245, 255), anchor="mm")
    return p.rotate(180) if upside_down else p


y = (big - half) // 2
img.alpha_composite(panel(True), (4 * SCALE, y))
img.alpha_composite(panel(False), (big - half - 4 * SCALE, y))

out = ROOT / "app/icon.png"
img.resize((SIZE, SIZE), Image.LANCZOS).save(out, optimize=True)
print(f"wrote {out}")
