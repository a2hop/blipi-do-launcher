# blipi-do-launcher
Blipi action launcher for Linux

## Description

BliPi Do is a GTK-based action launcher for Linux that provides a simple, graphical interface for executing predefined commands on selected files or directories. It's designed to be invoked from file managers or desktop environments to perform quick actions on files.

## Features

- **XML-based configuration**: Define custom actions and options in a simple XML format
- **Hierarchical menu**: Organize actions in a tree structure for easy navigation
- **File/directory support**: Pass selected files or directories as arguments to commands
- **Real-time output**: View command execution output directly in the application
- **Keyboard shortcuts**: Press ESC to close the window
- **GTK3 interface**: Native-looking, modern Linux GUI

## How It Works

1. The application reads actions from `/etc/blipi-do/actions.xml`
2. Displays available actions in a tree view
3. When you select an action, it executes the associated command with any files/directories passed as arguments
4. Shows the command output and exit status in real-time

## Installation

### From Source

1. Install dependencies:
   ```bash
   sudo apt-get install build-essential libgtk-3-dev libxml2-dev pkg-config
   ```

2. Build the application:
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

3. Create the configuration directory and copy the actions file:
   ```bash
   sudo mkdir -p /etc/blipi-do
   sudo cp actions.xml /etc/blipi-do/
   ```

4. Install the binary:
   ```bash
   sudo cp build/blipi-do /usr/local/bin/
   ```

## Configuration

Actions are defined in `/etc/blipi-do/actions.xml`. Example structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Actions>
    <Item label="Compress">
        <Item label="Create tar.gz" action="tar" options="czf;archive.tar.gz" />
        <Item label="Create zip" action="zip" options="r;archive.zip" />
    </Item>
    <Item label="Convert" action="convert" />
</Actions>
```

## Usage

Run the application with file or directory arguments:

```bash
blipi-do /path/to/file1 /path/to/file2
```

The selected files/directories will be passed as arguments to the executed action command.

