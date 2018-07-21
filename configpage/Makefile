NODE_MODULES := node_modules/.installed

dist/configpage.js: dist/index-embed.html
	echo -n "module.exports = '" > $@
	base64 -w0 $< >> $@
	echo "'" >> $@

dist/index-embed.html: dist/index.html embed.txr
	txr embed.txr $< > $@

dist/index.html: index.html app.js $(NODE_MODULES)
	node_modules/.bin/parcel build $<

$(NODE_MODULES): package.json
	npm install
	touch $@
