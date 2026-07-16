// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import lucode from 'lucode-starlight';
import { cleanHeadingSlugs } from './src/plugins/clean-heading-slugs.mjs';

// https://astro.build/config
export default defineConfig({
	integrations: [
		cleanHeadingSlugs(),
		starlight({
			title: 'WinSparkle',
			lastUpdated: true,
			customCss: ['./src/styles/custom.css'],
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
					items: [{ autogenerate: { directory: 'guides' } }],
				},
				{
					label: 'C API',
					items: [{ autogenerate: { directory: 'c-api' } }],
				},
				{
					label: 'Appendix',
					items: [{ autogenerate: { directory: 'appendix' } }],
				},
			],
		}),
	],
});
