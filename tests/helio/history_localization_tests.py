"""Check the compiled catalogs shipped with the application, not temporary .mo files."""

import argparse
import gettext
from pathlib import Path
import unittest


LOCALE_DIR = Path(__file__).resolve().parents[2] / "resources" / "i18n"
HISTORY_STRINGS = {
    "Download Preview G-code": "下载预览 G-code",
    "Download Print G-code": "下载打印 G-code",
    "Save Preview G-code": "保存预览 G-code",
    "Save Print G-code": "保存打印 G-code",
    "G-code for printing.": "用于打印的 G-code。",
    "G-code with analysis annotations for preview.": "带有分析标注、用于预览的 G-code。",
}


class HistoryLocalizationTests(unittest.TestCase):
    def test_packaged_history_translations(self):
        for language in ("zh_CN", "en"):
            with (LOCALE_DIR / language / "BambuStudio.mo").open("rb") as stream:
                catalog = gettext.GNUTranslations(stream)
            for english, chinese in HISTORY_STRINGS.items():
                with self.subTest(language=language, text=english):
                    # English would look correct even if its entry were absent.
                    self.assertIn(english, catalog._catalog)
                    self.assertEqual(catalog.gettext(english), chinese if language == "zh_CN" else english)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--locale-dir", type=Path, default=LOCALE_DIR)
    args, test_args = parser.parse_known_args()
    LOCALE_DIR = args.locale_dir
    unittest.main(argv=[__file__, *test_args])
