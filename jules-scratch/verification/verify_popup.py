import asyncio
from playwright.async_api import async_playwright, expect

async def main():
    async with async_playwright() as p:
        browser = await p.chromium.launch()
        page = await browser.new_page()

        await page.goto("http://localhost:5173/")

        # Wait for the popup to be visible
        popup_locator = page.locator("div.full-page-cover")
        await expect(popup_locator).to_be_visible()

        # Take a screenshot of the popup
        await page.screenshot(path="jules-scratch/verification/verification.png")

        await browser.close()

if __name__ == "__main__":
    asyncio.run(main())