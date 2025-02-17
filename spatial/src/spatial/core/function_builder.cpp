#include "spatial/core/function_builder.hpp"
#include "duckdb/catalog/catalog_entry/function_entry.hpp"
#include "duckdb/main/extension_util.hpp"

namespace spatial {

namespace core {

static string RemoveIndentAndTrailingWhitespace(const char *text) {
	string result;
	// Skip any empty first newlines if present
	while (*text == '\n') {
		text++;
	}

	// Track indent length
	auto indent_start = text;
	while (isspace(*text) && *text != '\n') {
		text++;
	}
	auto indent_len = text - indent_start;
	while (*text) {
		result += *text;
		if (*text++ == '\n') {
			// Remove all indentation, but only if it matches the first line's indentation
			bool matched_indent = true;
			for (auto i = 0; i < indent_len; i++) {
				if (*text != indent_start[i]) {
					matched_indent = false;
					break;
				}
			}
			if (matched_indent) {
				text += indent_len;
			}
		}
	}

	// Also remove any trailing whitespace
	result.erase(result.find_last_not_of(" \n\r\t") + 1);
	return result;
}

void FunctionBuilder::Register(DatabaseInstance &db, const char *name, ScalarFunctionBuilder &builder) {
	// Register the function
	ExtensionUtil::RegisterFunction(db, std::move(builder.set));

	// Also add the parameter names. We need to access the catalog entry for this.
	auto &catalog = Catalog::GetSystemCatalog(db);
	auto transaction = CatalogTransaction::GetSystemTransaction(db);
	auto &schema = catalog.GetSchema(transaction, DEFAULT_SCHEMA);
	auto catalog_entry = schema.GetEntry(transaction, CatalogType::SCALAR_FUNCTION_ENTRY, name);
	if (!catalog_entry) {
		// This should not happen, we just registered the function
		throw InternalException("Function with name \"%s\" not found in FunctionBuilder::AddScalar", name);
	}

	auto &func_entry = catalog_entry->Cast<FunctionEntry>();

	// Insert all descriptions
	for (auto &desc : builder.descriptions) {

		desc.description = RemoveIndentAndTrailingWhitespace(desc.description.c_str());
		for (auto &ex : desc.examples) {
			ex = RemoveIndentAndTrailingWhitespace(ex.c_str());
		}

		func_entry.descriptions.push_back(desc);
	}

	if (!builder.tags.empty()) {
		func_entry.tags = std::move(builder.tags);
	}
}

} // namespace core

} // namespace spatial