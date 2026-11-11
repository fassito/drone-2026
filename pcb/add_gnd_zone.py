#!/usr/bin/env python3
"""Añade el plano de masa (GND) en B.Cu y lo rellena.

  flatpak run --command=python3 org.kicad.KiCad pcb/add_gnd_zone.py

El relleno respeta la holgura de las pistas ya enrutadas, así que se puede
ejecutar también al final del enrutado para refrescar la zona.
"""
import os

import pcbnew

PCB = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "drone-2026.kicad_pcb")
MARGIN = 0.5


def mm(v):
    return pcbnew.FromMM(v)


def main():
    board = pcbnew.LoadBoard(PCB)

    xs, ys = [], []
    for d in board.GetDrawings():
        if d.GetLayer() == pcbnew.Edge_Cuts:
            bb = d.GetBoundingBox()
            xs += [pcbnew.ToMM(bb.GetLeft()), pcbnew.ToMM(bb.GetRight())]
            ys += [pcbnew.ToMM(bb.GetTop()), pcbnew.ToMM(bb.GetBottom())]
    x0, y0, x1, y1 = min(xs), min(ys), max(xs), max(ys)

    net = board.FindNet("GND")
    if net is None:
        raise SystemExit("no existe la red GND")

    for z in list(board.Zones()):
        if z.GetNetname() == "GND":
            board.Remove(z)

    zone = pcbnew.ZONE(board)
    zone.SetLayer(pcbnew.B_Cu)
    zone.SetNet(net)
    zone.SetLocalClearance(mm(0.25))
    zone.SetMinThickness(mm(0.25))
    zone.SetPadConnection(pcbnew.ZONE_CONNECTION_THERMAL)
    zone.SetThermalReliefGap(mm(0.5))
    zone.SetThermalReliefSpokeWidth(mm(0.5))

    out = zone.Outline()
    out.NewOutline()
    for x, y in ((x0 + MARGIN, y0 + MARGIN), (x1 - MARGIN, y0 + MARGIN),
                 (x1 - MARGIN, y1 - MARGIN), (x0 + MARGIN, y1 - MARGIN)):
        out.Append(mm(x), mm(y))
    board.Add(zone)

    board.BuildConnectivity()
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    pcbnew.SaveBoard(PCB, board)
    print("plano GND en B.Cu: %.1f x %.1f mm" % (x1 - x0, y1 - y0))


if __name__ == "__main__":
    main()
