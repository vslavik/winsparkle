// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import lucode from 'lucode-starlight';

// https://astro.build/config
export default defineConfig({
	integrations: [
		starlight({
			title: 'WinSparkle',
			lastUpdated: true,
  			editLink: {
    			baseUrl: 'https://github.com/vslavik/winsparkle/edit/master/docs/',
  			},
            plugins: [
				lucode({
					footerText: '',
				}),
			],
			social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/vslavik/winsparkle' }],
			sidebar: [
				{
					label: 'Guides',
					items: [
						// Each item here is one entry in the navigation menu.
						{ label: 'Example Guide', slug: 'guides/example' },
					],
				},
				{
					label: 'Reference',
					items: [{ autogenerate: { directory: 'reference' } }],
				},
			],
		}),
	],
});
