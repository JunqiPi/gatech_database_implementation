# -*- coding: utf-8 -*-
"""Re-extract paper1.pdf (Codd 1970, two-column typeset) with coordinate-based
line reconstruction so words are reflowed into proper lines/columns."""
import os
import fitz

BASE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(BASE, "extracted")
SRC = os.path.join(BASE, "paper1.pdf")


def reflow_page(page, col_split):
    words = page.get_text("words")  # x0,y0,x1,y1,word,block,line,wordno
    if not words:
        return ""
    cols = [[w for w in words if w[0] < col_split],
            [w for w in words if w[0] >= col_split]]
    out_lines = []
    for col in cols:
        if not col:
            continue
        col.sort(key=lambda w: (round(w[1] / 4), w[0]))  # by y-band then x
        lines = []
        cur = []
        cur_y = None
        for w in col:
            y = w[1]
            if cur_y is None or abs(y - cur_y) <= 4:
                cur.append(w)
                cur_y = y if cur_y is None else cur_y
            else:
                lines.append(cur)
                cur = [w]
                cur_y = y
        if cur:
            lines.append(cur)
        for ln in lines:
            ln.sort(key=lambda w: w[0])
            out_lines.append(" ".join(w[4] for w in ln))
    text = "\n".join(out_lines)
    for k, v in {chr(0xF073): "σ", chr(0xF072): "ρ", chr(0xF0D5): "Π",
                 chr(0xF0C8): "∪", chr(0xF0C7): "∩", chr(0xF0CD): "⊆",
                 chr(0xF0AC): "←", chr(0xF0E8): "→"}.items():
        text = text.replace(k, v)
    return text


def main():
    doc = fitz.open(SRC)
    page_w = doc[0].rect.width
    split = page_w / 2  # ~279
    lines = ["# paper1", "",
             "_(双栏排版，已按坐标重排为左栏→右栏的阅读顺序)_", ""]
    for i, page in enumerate(doc, 1):
        lines.append(f"## Page {i}")
        lines.append("")
        lines.append(reflow_page(page, split).rstrip())
        lines.append("")
    out_path = os.path.join(OUT, "paper1.md")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines).rstrip() + "\n")
    print("rewrote", out_path, "pages:", doc.page_count)


if __name__ == "__main__":
    main()
