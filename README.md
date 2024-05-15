# UOS-AI with suport for ollama and gemini

UOS-AI is a fork of uos-ai deepin assistant for Deepin 20.9, with support for ollama local models and gemini.

## Installation

### Build from source code

To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -us -uc -b # build binary package(s)
```

## Usage
### Ollama
1. You need to have running your ollama service.
Open this url in your browser [http://localhost:11434](http://localhost:11434), should show "Ollama is running".
2. Open uos-ai and add the model: 
* Account: local model 
* LLM: Ollama
* APIKey: anything

### Gemini
* Account: gemini-pro (only tested with this model)
* LLM: Gemini
* APIKey: gemini key

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.
