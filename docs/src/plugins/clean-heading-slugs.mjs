function cleanSlug(slug) {
	return slug.includes('_') && slug.includes('-') ? slug.replaceAll('-', '') : slug;
}

function slugify(text) {
	return text
		.toLowerCase()
		.replace(/[^\w\s-]/g, '')
		.replace(/\s+/g, '-');
}

function cleanHeadingSlug(node, ctx) {
	const id = node.properties?.id;
	const slug = typeof id === 'string' ? id : slugify(ctx.textContent(node));

	const cleaned = cleanSlug(slug);
	if (cleaned === slug) return;

	if (ctx?.setProperty) {
		ctx.setProperty(node, 'id', cleaned);
	} else {
		node.properties.id = cleaned;
	}
}

function cleanHeadingSlugsPlugin() {
	return {
		name: 'winsparkle-clean-heading-slugs',
		element: [
			{
				filter: ['h1', 'h2', 'h3', 'h4', 'h5', 'h6'],
				visit: cleanHeadingSlug,
			},
		],
	};
}

export function cleanHeadingSlugs() {
	return {
		name: 'winsparkle-clean-heading-slugs',
		hooks: {
			'astro:config:setup'({ config, logger }) {
				const processor = config.markdown?.processor;

				if (processor?.name !== 'satteri') {
					logger.warn('Skipping heading slug cleanup: expected the Satteri Markdown processor.');
					return;
				}

				processor.options.hastPlugins.push(cleanHeadingSlugsPlugin());
			},
		},
	};
}
