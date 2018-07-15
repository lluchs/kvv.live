SRC := $(shell find src)

default:
	@echo "How-to:"
	@echo " 1. run make gen_dict"
	@echo " 2. update locale/locale_german.json based on locale_english.json"
	@echo " 3. run make dict2bin"

locale/locale_english.json: $(SRC)
	python2 locale/gen_dict.py src/ $@
	mv locale/locale_english.bin resources

resources/locale_german.bin: locale/locale_german.json
	python2 locale/dict2bin.py $<
	mv locale/locale_german.bin $@

gen_dict: locale/locale_english.json
dict2bin: resources/locale_german.bin

.PHONY: default gen_dict dict2bin
