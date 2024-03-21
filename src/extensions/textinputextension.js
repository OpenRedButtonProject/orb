(function() {
    function onTextInputFocused(e) {
        hbbtv.native.request('SoftKeyboard.show');
    }
    const observer = new MutationObserver(function(mutationsList) {
        for (const mutation of mutationsList) {
            for (const node of mutation.removedNodes) {
                if (node.nodeName && node.nodeName.toLowerCase() === 'input') {
                    node.removeEventListener('focus', onTextInputFocused);
                }
            }
            for (const node of mutation.addedNodes) {
                if (node.nodeName && node.nodeName.toLowerCase() === 'input' && node.type === 'text') {
                    node.addEventListener('focus', onTextInputFocused);
                }
            }
        }
    });
    const config = {
        childList: true,
        subtree: true,
    };
    observer.observe(document.documentElement || document.body, config);
})();