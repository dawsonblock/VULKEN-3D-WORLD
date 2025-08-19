from scripts.perform_merge import copytree


def test_copytree_requires_confirmation(tmp_path, monkeypatch):
    src = tmp_path / "src"
    dst = tmp_path / "dst"
    src.mkdir()
    (src / "file").write_text("data")
    dst.mkdir()
    old_file = dst / "old"
    old_file.write_text("old")

    monkeypatch.setattr("builtins.input", lambda _: "n")
    copytree(src, dst, True)
    assert old_file.exists()

    monkeypatch.setattr("builtins.input", lambda _: "y")
    copytree(src, dst, True)
    assert (dst / "file").exists()
    assert not old_file.exists()

