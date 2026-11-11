#!/usr/bin/env python3
"""Pre-coloca la placa drone-2026 con la API pcbnew de KiCad.

Genera drone-2026.kicad_pcb con:
  - contorno cuadrado (lado elegido automáticamente) en Edge.Cuts
  - 4 agujeros M3 en patrón 30.5x30.5 mm (estándar de quads de 5")
  - las 33 huellas colocadas sin solapes ni invasión de los agujeros
  - las redes asignadas a cada pad

Debe ejecutarse con el intérprete de KiCad:
  flatpak run --command=python3 org.kicad.KiCad pcb/pcb_layout.py
"""
import os
import sys

import pcbnew

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import generate_schematic as gs  # noqa: E402  (solo datos)

FP_DIR = "/app/extensions/Library/Footprints/footprints"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "drone-2026.kicad_pcb")

MARGIN = 2.0      # margen libre desde el borde de la placa
GAP = 1.6         # separación entre huellas
HOLE_CLEAR = 1.5  # holgura alrededor de cada agujero M3


def mm(v):
    return pcbnew.FromMM(v)


def box_hits_circle(bx, by, bw, bh, cx, cy, r):
    """True si el rectángulo toca el círculo."""
    nx = min(max(cx, bx), bx + bw)
    ny = min(max(cy, by), by + bh)
    return (nx - cx) ** 2 + (ny - cy) ** 2 < r * r


def pack(items, side, holes):
    """Coloca por estanterías esquivando los agujeros."""
    limit = side - MARGIN
    x, y, shelf_h = MARGIN, MARGIN, 0.0
    pos = {}
    for it in items:
        w, h = it["w"], it["h"]
        while True:
            if x + w > limit:               # fin de estantería
                x = MARGIN
                y += shelf_h
                shelf_h = 0.0
            if y + h > limit:               # no cabe
                return False, pos
            clash = any(box_hits_circle(x - GAP, y - GAP, w + 2 * GAP,
                                        h + 2 * GAP, hx, hy, r)
                        for hx, hy, r in holes)
            if not clash:
                break
            x += 1.0
        pos[it["ref"]] = (x, y)
        x += w + GAP
        shelf_h = max(shelf_h, h + GAP)
    return True, pos


def nets_by_pad_number(lib, sym, netmap):
    """Traduce un mapa cuyas claves pueden ser nombres de pin a números."""
    if all(k.isdigit() for k in netmap):
        return {k: v for k, v in netmap.items()}
    raw = gs.load_symbol(lib, sym)
    name_to_num = {nm: num for num, nm, _x, _y, _a in gs.parse_pins(raw)}
    out = {}
    for key, net in netmap.items():
        num = key if key.isdigit() else name_to_num.get(key)
        if num is not None:
            out[num] = net
    return out


def main():
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(2)

    nets = {}
    for _, _, _, _, _, _, netmap in gs.COMPONENTS:
        for net in netmap.values():
            if net and net not in nets:
                item = pcbnew.NETINFO_ITEM(board, net)
                board.Add(item)
                nets[net] = item

    items = []
    for ref, value, lib, sym, _c, _r, netmap in gs.COMPONENTS:
        fpname = gs.footprint_for(lib, sym)
        libnick, name = fpname.split(":")
        fp = pcbnew.FootprintLoad(os.path.join(FP_DIR, libnick + ".pretty"), name)
        if fp is None:
            raise SystemExit("no se pudo cargar %s" % fpname)
        fp.SetReference(ref)
        fp.SetValue(value)
        board.Add(fp)
        # el texto del valor no se muestra en la placa (evita solapes de silk)
        fp.Value().SetVisible(False)
        fp.SetPosition(pcbnew.VECTOR2I(0, 0))
        bb = fp.GetBoundingBox(False, False)      # sin textos
        items.append({"fp": fp, "ref": ref,
                      "w": pcbnew.ToMM(bb.GetWidth()),
                      "h": pcbnew.ToMM(bb.GetHeight()),
                      # desplazamiento del bbox respecto al origen de la huella
                      "dx": pcbnew.ToMM(bb.GetLeft()),
                      "dy": pcbnew.ToMM(bb.GetTop())})
    items.sort(key=lambda d: d["w"] * d["h"], reverse=True)

    side = None
    for candidate in range(50, 106, 5):
        holes = [(candidate / 2 + dx, candidate / 2 + dy, 1.6 + HOLE_CLEAR)
                 for dx, dy in ((-15.25, -15.25), (15.25, -15.25),
                                (-15.25, 15.25), (15.25, 15.25))]
        ok, pos = pack(items, candidate, holes)
        if ok:
            side = candidate
            break
    if side is None:
        raise SystemExit("no cabe ni en 105x105 mm")
    print("lado elegido: %d mm" % side)

    corners = [(0, 0), (side, 0), (side, side), (0, side)]
    for i in range(4):
        x1, y1 = corners[i]
        x2, y2 = corners[(i + 1) % 4]
        s = pcbnew.PCB_SHAPE(board)
        s.SetShape(pcbnew.SHAPE_T_SEGMENT)
        s.SetStart(pcbnew.VECTOR2I(mm(x1), mm(y1)))
        s.SetEnd(pcbnew.VECTOR2I(mm(x2), mm(y2)))
        s.SetLayer(pcbnew.Edge_Cuts)
        s.SetWidth(mm(0.1))
        board.Add(s)

    for i, (hx, hy, _r) in enumerate(holes):
        fp = pcbnew.FootprintLoad(
            os.path.join(FP_DIR, "MountingHole.pretty"),
            "MountingHole_3.2mm_M3")
        fp.SetPosition(pcbnew.VECTOR2I(mm(hx), mm(hy)))
        fp.SetReference("H%d" % (i + 1))
        board.Add(fp)

    for it in items:
        bx, by = pos[it["ref"]]           # esquina superior izquierda deseada
        it["fp"].SetPosition(pcbnew.VECTOR2I(mm(bx - it["dx"]),
                                             mm(by - it["dy"])))

    netmaps = {ref: nets_by_pad_number(lib, sym, netmap)
               for ref, _v, lib, sym, _c, _r, netmap in gs.COMPONENTS}
    for it in items:
        for pad in it["fp"].Pads():
            name = netmaps[it["ref"]].get(pad.GetNumber())
            if name:
                pad.SetNet(nets[name])

    boxes = []
    for it in items:
        bb = it["fp"].GetBoundingBox(False, False)
        boxes.append((it["ref"], pcbnew.ToMM(bb.GetLeft()),
                      pcbnew.ToMM(bb.GetTop()),
                      pcbnew.ToMM(bb.GetWidth()),
                      pcbnew.ToMM(bb.GetHeight())))
    for i, (hx, hy, _r) in enumerate(holes):
        boxes.append(("H%d" % (i + 1), hx - 3.2, hy - 3.2, 6.4, 6.4))

    clashes = []
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            _, x1, y1, w1, h1 = boxes[i]
            _, x2, y2, w2, h2 = boxes[j]
            if x1 < x2 + w2 and x2 < x1 + w1 and y1 < y2 + h2 and y2 < y1 + h1:
                clashes.append((boxes[i][0], boxes[j][0]))
    outside = [b[0] for b in boxes
               if b[1] < 0 or b[2] < 0
               or b[1] + b[3] > side or b[2] + b[4] > side]

    print("huellas: %d" % len(items))
    print("solapes: %s" % (clashes if clashes else "ninguno"))
    print("fuera del contorno: %s" % (outside if outside else "ninguna"))

    pcbnew.SaveBoard(OUT, board)
    print("guardado -> %s" % OUT)


if __name__ == "__main__":
    main()
