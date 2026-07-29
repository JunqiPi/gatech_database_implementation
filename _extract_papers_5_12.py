# -*- coding: utf-8 -*-
"""Extract raw/Raw_papers/paper5..12.pdf into extracted/paperN.md.

Most DB papers are two-column typeset, so extraction is coordinate-based:
words are first reflowed into full-page lines, each line is then classified as
full-width (title/abstract banner/footnote rule) or left/right column, and the
page is emitted in true reading order: full-width lines act as separators, and
between them the left column is emitted before the right one.

paper9.pdf has no text layer (scanned TIFF->PDF); it is rendered to PNGs under
extracted/paper9_pages/ for visual reading instead.
"""
import os
import fitz

BASE = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(BASE, "raw", "Raw_papers")
OUT = os.path.join(BASE, "extracted")

SYMBOL_MAP = {
    chr(0xF073): "σ", chr(0xF072): "ρ", chr(0xF0D5): "Π",
    chr(0xF0C8): "∪", chr(0xF0C7): "∩", chr(0xF0CD): "⊆",
    chr(0xF0AC): "←", chr(0xF0E8): "→", chr(0xF02A): "×",
}


def band_rows(words, band=3.5):
    """Group words into visual rows by y-band; each row sorted left to right."""
    words = sorted(words, key=lambda w: (w[1], w[0]))
    rows, cur, cur_y = [], [], None
    for w in words:
        if cur_y is None or abs(w[1] - cur_y) <= band:
            cur.append(w)
            cur_y = w[1] if cur_y is None else cur_y
        else:
            rows.append(sorted(cur, key=lambda x: x[0]))
            cur, cur_y = [w], w[1]
    if cur:
        rows.append(sorted(cur, key=lambda x: x[0]))
    return rows


def mk(seg):
    return {"x0": min(w[0] for w in seg), "x1": max(w[2] for w in seg),
            "y": min(w[1] for w in seg),
            "text": " ".join(w[4] for w in seg)}


def split_at_gutter(row, split, min_gap=12.0):
    """A y-band row may hold one full-width line OR one line from each column.
    Split it only if there is a wide blank gap straddling the page midpoint."""
    if row[0][0] >= split or row[-1][2] <= split:
        return [mk(row)]  # entirely on one side
    for i in range(len(row) - 1):
        gap_l, gap_r = row[i][2], row[i + 1][0]
        if gap_r - gap_l >= min_gap and gap_l <= split <= gap_r:
            return [mk(row[:i + 1]), mk(row[i + 1:])]
    return [mk(row)]  # genuinely spans the gutter -> title / banner


def page_text(page):
    words = page.get_text("words")
    if not words:
        return ""
    split = page.rect.width / 2
    rows = band_rows(words)
    lines = []
    for r in rows:
        lines.extend(split_at_gutter(r, split))
    if len(lines) < 8:
        return "\n".join(l["text"] for l in lines)

    def side(l):
        if l["x1"] <= split + 6:
            return "L"
        if l["x0"] >= split - 6:
            return "R"
        return "F"  # full width

    tags = [side(l) for l in lines]
    n_l = tags.count("L")
    n_r = tags.count("R")
    # Not a two-column page: one side barely used.
    if min(n_l, n_r) < 0.2 * len(lines):
        text = "\n".join(l["text"] for l in lines)
    else:
        # Reading order: full-width lines separate blocks; within a block the
        # whole left column precedes the whole right column.
        out, buf_l, buf_r = [], [], []

        def flush():
            out.extend(buf_l)
            out.extend(buf_r)
            buf_l.clear()
            buf_r.clear()

        for l, t in zip(lines, tags):
            if t == "F":
                flush()
                out.append(l["text"])
            elif t == "L":
                buf_l.append(l["text"])
            else:
                buf_r.append(l["text"])
        flush()
        text = "\n".join(out)

    for k, v in SYMBOL_MAP.items():
        text = text.replace(k, v)
    return text


def render_scanned(src, name, dpi=200):
    """No text layer -> render each page to PNG for visual reading."""
    doc = fitz.open(src)
    outdir = os.path.join(OUT, f"{name}_pages")
    os.makedirs(outdir, exist_ok=True)
    for i, page in enumerate(doc, 1):
        pix = page.get_pixmap(dpi=dpi)
        pix.save(os.path.join(outdir, f"page{i:02d}.png"))
    return outdir, doc.page_count


def main():
    for n in range(5, 13):
        src = os.path.join(SRC_DIR, f"paper{n}.pdf")
        if not os.path.exists(src):
            print("missing", src)
            continue
        name = f"paper{n}"
        doc = fitz.open(src)
        has_text = any(p.get_text("words") for p in doc)
        if not has_text:
            outdir, pages = render_scanned(src, name)
            print(f"{name}.pdf -> NO TEXT LAYER, rendered {pages} PNGs to {outdir}")
            continue
        lines = [f"# {name}", "",
                 f"_(共 {doc.page_count} 页，双栏按坐标重排为左栏→右栏；跨栏标题保持原位)_", ""]
        for i, page in enumerate(doc, 1):
            lines.append(f"## Page {i}")
            lines.append("")
            lines.append(page_text(page).rstrip())
            lines.append("")
        out_path = os.path.join(OUT, f"{name}.md")
        with open(out_path, "w", encoding="utf-8") as f:
            f.write("\n".join(lines).rstrip() + "\n")
        print(f"{name}.pdf -> {name}.md ({doc.page_count} pages)")


if __name__ == "__main__":
    main()
