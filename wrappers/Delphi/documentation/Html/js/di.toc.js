/* 
	Documentation Insight
	Copyright (C) 2010-2013 DevJet Software
*/

var isIndexTabInitialized = false;
var activeHelpId;	
var isSyncIndex = false;

function encodeHtml(value){
  if (value) {
    return $('<div />').text(value).html();
  } else {
    return '';
  }
}

function decodeHtml(value) {
  if (value) {
    return $('<div />').html(value).text();
  } else {
    return '';
  }
}

function selectTocItem(helpId, autoScroll)
{
	var autoScroll = (typeof autoScroll === "undefined") ? true : autoScroll;

	if ($("#tab-toc").length) {
		var node = $("#tab-toc").dynatree("getTree").getNodeByKey(helpId);
		var activeNode = $("#tab-toc").dynatree("getActiveNode");
		if (node == activeNode) 
			return;
		node.activateSilently();
		if (autoScroll)
			scrollToNode($('#tab-toc'), node);		
	}
	else {
		activeHelpId = helpId;
	}
}

function scrollToNode(tree, node) {
	var activeLi = node.li;
	var nodeOffsetTop = $(activeLi).offset().top;
		tree.animate({
			scrollTop: $(activeLi).offset().top - tree.offset().top + tree.scrollTop()
		}, 'fast');	
}		

function findNode(tree, text) {
	var content = encodeHtml(text);

	tree.dynatree('getRoot').visit(
		function(node) {
			if (node.data.title.substring(0, content.length).toLowerCase()==content.toLowerCase())
			{
				node.activateSilently();
				scrollToNode(tree, node);
				return false;
			}
		},
		false
	);
}

function syncIndexEntry(node) {			
	isSyncIndex = true;
	try {
		var title = $("<div/>").html(node.data.title).text(); // Decode html entities.
		$('#searchBox').val(title);
	}
	finally
	{
		isSyncIndex = false
	}
}

$(function()
{					
	$("#tabs").tabs({
		selected: 0,
		activate: function(event, ui) {
			if (ui.newPanel.attr('id').localeCompare('tab-index')==0) {
				$("#searchLayout").show();
				$("#searchBox").focus().select();
				
				if (!isIndexTabInitialized) {
					isIndexTabInitialized = true;
					$("#tab-index").dynatree({
						clickFolderMode: 1,
						selectMode: 1,
						autoFocus: false,
						debugLevel: 0,
						imagePath: 'css/dynatree/',
						initAjax: {url: 'keywords.js'},
						onPostInit: function(isReloading, isError) {
							$('#searchBox').focus();
						},
						fx: { height: "toggle", duration: 200 },
						onActivate: function(node){
							if(node.data.href && node.data.href != '#'){
								window.open(node.data.href, "contentFrame");					
								syncIndexEntry(node);
							}
						}
					});
				}
			}
			else {
				$("#searchLayout").hide()
			}
		}
	});
	
	$('#tab-index').css('top', 100);
	
	$("#tab-toc").dynatree({
		clickFolderMode: 1,
		debugLevel: 0,
		selectMode: 1,
		imagePath: 'css/dynatree/',
		initAjax: {url: 'toc.js'},
		fx: { height: "toggle", duration: 200 },
		onActivate: function(node){
			if(node.data.href && node.data.href != '#'){
				window.open(node.data.href, "contentFrame");
			}					
			else {
				node.expand(true);
			}
		},
		onDblClick: function(node, event) {
			node.expand(! node.isExpanded());
		}
	});		
	
	$('#searchBox').on('textchange', 
		function (event, previousText) {
			if (isSyncIndex) return;								

			findNode($('#tab-index'), this.value);
			return;					
		}
	).keydown(
		function(event){
			var activeNode = $('#tab-index').dynatree('getActiveNode');
			var handled = true;
			var node = null;

			if (activeNode) {
				switch (event.which) {
					case 38: // Arrow UP
						node = activeNode.getPrevSibling();
						if (node && node.getLevel() != 0) 
						{
							node.activateSilently();
							syncIndexEntry(node);
						}
						break;
						
					case 40: // Arrow Down
						node = activeNode.getNextSibling();
						if (node) 
						{
							node.activateSilently();
							syncIndexEntry(node);
						}								
						break;
					case 13: // Enter
						if(activeNode.data.href && activeNode.data.href != '#'){
							window.open(activeNode.data.href, "contentFrame");
							syncIndexEntry(activeNode);
						}
						break;
					case 27: // Escape
						$(this).val('');
						break;
					default:
						handled = false;
				}
				if (handled) { 
					event.preventDefault();
					this.select();
				}
			}
	}).mousedown(function(){
		if (!$(this).is(':focus')) 
		{
			$(this).focus();
			$(this).select();
			return false;
		}
	});
});
