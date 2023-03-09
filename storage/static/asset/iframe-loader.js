function loadPage() {
  let url = document.getElementById('page-url').value;
  url = url.replace('/storage/static/','');
  if (!url.startsWith("http")) {
    url = "http://" + url;
  }
  alert(url);
  var iframe = document.createElement('iframe');
  iframe.setAttribute('src', url);
  iframe.style.width = '100%';
  iframe.style.height = '500px';
  document.getElementById('iframe-container').appendChild(iframe);
}
