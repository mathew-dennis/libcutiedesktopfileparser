# libcutiedesktopfileparser
[![CircleCI](https://dl.circleci.com/status-badge/img/gh/cutie-shell/libcutiedesktopfileparser/tree/cutie.svg?style=svg)](https://dl.circleci.com/status-badge/redirect/gh/cutie-shell/libcutiedesktopfileparser/tree/cutie)

This library provides desktop file parser capability for Cutie.

## Building and installing

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Using Functions

### `fetchAllEntriesModel()`

Returns a flat model of all applications. There are two ways to call it:

1. Default application locations — fetches all installed applications from the standard application directories:

```qml
ListView {
    model: CutieDesktopFileParser.fetchAllEntriesModel()
    delegate: Text { text: name }
}
```

2. Custom paths (e.g., autostart) — fetches desktop entries from specific directories:

```qml
ListView {
    model: CutieDesktopFileParser.fetchAllEntriesModel(["/etc/xdg/autostart", "~/.config/autostart"])
    delegate: Text { text: name }
}
```

Each call returns a separate, independent model, so you can use multiple lists simultaneously (Settings apps, App menu, Autostart apps).

Each entry exposes the following roles:

| Role | Type | Description |
|---|---|---|
| `name` | string | Application display name |
| `exec` | string | Command to launch the app |
| `icon` | string | Icon name or path |
| `entry` | map | Full raw entry map |

---

### `createFilterModel()`

Returns an `AppFilterProxyModel` — a filterable proxy model wrapping the full app list. Best used when the list needs to be filtered dynamically.

Supports the same path arguments as `fetchAllEntriesModel()`:

```qml
property var appModel: CutieDesktopFileParser.createFilterModel()
// or with custom paths:
property var appModel: CutieDesktopFileParser.createFilterModel(["/etc/xdg/autostart"])
```

#### Filtering by favorites

Set `favoriteKeys` to show only specific apps. Setting it to an empty list disables the filter and shows all apps:

```qml
appModel.favoriteKeys = Object.keys(favoriteStore.data)
```

#### Filtering by search

Set `searchQuery` to filter apps by name, case-insensitive. The list updates instantly as the query changes:

```qml
TextField {
    onTextChanged: appModel.searchQuery = text
}
```

#### Combining both filters

Both filters can be active at the same time — an app must pass both to appear in the list:

```qml
property var appModel: CutieDesktopFileParser.createFilterModel()

// only favorites
appModel.favoriteKeys = Object.keys(favoriteStore.data)

// live search within favorites
TextField {
    onTextChanged: appModel.searchQuery = text
}

ListView {
    model: appModel
    delegate: Text { text: name }
}
```

---

## Troubleshooting

This library includes `qDebug()` statements to help track its execution. When fetching entries, the debug output will show:

- Which directories are being scanned
- Which desktop files are read
- The total number of entries found

If your app list appears empty, check the debug output to verify:

- The correct directories are being scanned
- `.desktop` files are present and not marked `Hidden=true` or `NoDisplay=true`
