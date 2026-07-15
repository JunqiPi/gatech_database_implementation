# -*- coding: utf-8 -*-
"""Combine all extracted .md into one file + build a README index."""
import os, glob

BASE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(BASE, "extracted")

ORDER = ["ch1", "ch2", "ch12", "ch13", "paper1", "paper2", "paper3", "paper4"]
TITLES = {
    "ch1": "Chapter 1 — Introduction (课件)",
    "ch2": "Chapter 2 — Relational Model / Relational Algebra (课件)",
    "ch12": "Chapter 12 — Physical Storage Systems (课件)",
    "ch13": "Chapter 13 — Data Storage Structures (课件)",
    "paper1": "Paper 1 — E. F. Codd, 'A Relational Model of Data for Large Shared Data Banks' (1970)",
    "paper2": "Paper 2 — Stonebraker & Pavlo, 'What Goes Around Comes Around... And Around...'",
    "paper3": "Paper 3 — Michael Stonebraker, 'Operating System Support for Database Management' (1981)",
    "paper4": "Paper 4 — Neumann & Freitag, 'Umbra: A Disk-Based System with In-Memory Performance'",
}


def count(md):
    t = open(md, encoding="utf-8").read()
    slides = t.count("\n## Slide ")
    pages = t.count("\n## Page ")
    words = len(t.split())
    return slides, pages, words


def main():
    combined = ["# Database Midterm 1 — 全部资料文本提取（合并版）",
                "",
                "> 本文件由 `Database_midterm_1/` 下的 4 个 PPT 与 4 篇 PDF 论文逐字提取、合并而成。",
                "> 适合一次性粘贴给任意 AI 阅读。每份资料以一级标题 `#` 分隔，幻灯片用 `## Slide N`、论文用 `## Page N`。",
                "",
                "---", ""]
    manifest = []
    for key in ORDER:
        md = os.path.join(OUT, key + ".md")
        if not os.path.exists(md):
            continue
        s, p, w = count(md)
        unit = f"{s} slides" if s else f"{p} pages"
        manifest.append((key, TITLES.get(key, key), unit, w))
        body = open(md, encoding="utf-8").read()
        combined.append(f"# === {TITLES.get(key, key)} ===")
        combined.append("")
        # drop the file's own top "# key" line to avoid double heading
        lines = body.splitlines()
        if lines and lines[0].startswith("# "):
            lines = lines[1:]
        combined.append("\n".join(lines).strip())
        combined.append("")
        combined.append("---")
        combined.append("")

    with open(os.path.join(OUT, "_ALL_IN_ONE.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(combined).rstrip() + "\n")

    # README
    r = ["# Database Midterm 1 — 提取说明 (README)",
         "",
         "原始资料：`Database_midterm_1/` 下的 4 个 PowerPoint 课件 + 4 篇 PDF 论文。",
         "本目录 `extracted/` 是逐字（verbatim）提取出的纯文本 Markdown，便于任何 AI 阅读和检索。",
         "",
         "## 文件清单", "",
         "| 文件 | 内容 | 规模 | 词数(约) |",
         "| --- | --- | --- | --- |"]
    for key, title, unit, w in manifest:
        r.append(f"| `{key}.md` | {title} | {unit} | {w:,} |")
    r += ["",
          "另有 `_ALL_IN_ONE.md`：以上全部内容合并为单一文件，方便一次性喂给 AI。",
          "",
          "## 格式约定", "",
          "- 每份资料以 `# 文件名` 开头。",
          "- PPT 的每页为 `## Slide N`；PDF 论文的每页为 `## Page N`。",
          "- PPT 演讲者备注（若有）以 `> [备注 Notes]` 引用块标出。",
          "- 表格转为 Markdown 表格。",
          "",
          "## 提取处理说明", "",
          "- **关系代数符号已还原**：PowerPoint 把 Symbol/Wingdings 字体的符号存为私用区编码"
          "（U+F0xx），已统一映射回真实 Unicode：σ(select)、Π(project)、∪(union)、∩(intersection)、"
          "ρ(rename)、⊆、≤、≥、≠、∧(and)、∨(or)、¬(not)、←(赋值)、→(results in)、×(乘)。",
          "- **paper1.pdf**（Codd 1970，老式双栏排版）按文字坐标重排为「左栏→右栏」的正常阅读顺序；"
          "其余 PDF 为现代单/双栏，文本流本身正确，直接提取。",
          "- **固有限制**：纯图片的幻灯片（如示意图、查询结果表截图）没有可提取的文字，"
          "对应 `## Slide N` 下会是空的——这是图片内容，非提取遗漏。",
          ""]
    with open(os.path.join(OUT, "README.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(r).rstrip() + "\n")

    print("manifest:")
    for m in manifest:
        print(" ", m)
    print("wrote _ALL_IN_ONE.md and README.md")


if __name__ == "__main__":
    main()
