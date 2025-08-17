import json
from pathlib import Path


def test_basic_palette():
    path = Path('assets/worldgen/palettes/basic.json')
    data = json.loads(path.read_text())
    assert data["name"] == "basic"
    assert "blocks" in data and data["blocks"], "Palette must define blocks"


def test_structure_templates():
    templates = ['tree.json', 'house.json']
    for tpl in templates:
        path = Path('assets/worldgen/templates') / tpl
        data = json.loads(path.read_text())
        assert "structure" in data and data["structure"], f"{tpl} should define structure"
