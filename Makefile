.PHONY: build test static sanitize coverage fuzz-clean campaign demo
build:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build -j 2
test: build
	ctest --test-dir build --output-on-failure
static:
	bash scripts/static_analysis.sh
sanitize:
	bash scripts/sanitize.sh
coverage:
	bash scripts/coverage.sh
campaign: build
	python3 scripts/evaluate_fault_campaign.py --binary ./build/cansec --output artifacts/fault_campaign.json
demo: build
	./build/cansec analyze --trace fixtures/nominal.csv --rules config/rules.csv --signals config/signals.csv
fuzz-clean:
	rm -rf build-fuzz
